#include "../../../include/libdane/platform/filesystem.h"

#include <ctype.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_BUFFER_MAX (LIBD_PF_FS_PATH_MAX + 1)
#define WORK_BUFFER_MAX (2 * LIBD_PF_FS_PATH_MAX + 1)

struct libd_platform_filesystem_path_s {
  size_t length;
  char path[PATH_BUFFER_MAX];
};

typedef libd_platform_filesystem_result_e result_e;
typedef libd_platform_filesystem_path_o path_o;
typedef libd_platform_filetype_path_type_e path_type_e;
typedef libd_platform_filesystem_env_getter_f env_getter_f;

result_e
_expand_path_env_variables(char dest[WORK_BUFFER_MAX],
                           const char* path,
                           env_getter_f env_getter);
result_e
_normalize_path_string(char dest_path[WORK_BUFFER_MAX],
                       const char src_path[WORK_BUFFER_MAX],
                       path_type_e path_type);

static inline bool
_is_valid_portable_path_char(const char c);

static inline bool
_is_valid_to_follow_period(const char c);

static inline char*
_find_char_or_end(char* s, char c);

result_e
libd_platform_filesystem_path_init(path_o* out_path,
                                   const char* raw_path,
                                   size_t raw_path_length_bytes,
                                   path_type_e type,
                                   env_getter_f env_getter)
{
  if (out_path == NULL || raw_path == NULL) {
    return LIBD_PF_FS_NULL_PARAMETER;
  }
  out_path->path[0] = '\0';

  char expansion_buffer[WORK_BUFFER_MAX];
  result_e result =
    _expand_path_env_variables(expansion_buffer, raw_path, env_getter);
  if (result != LIBD_PF_FS_OK) {
    return result;
  }

  char normalization_buffer[WORK_BUFFER_MAX];
  result = _normalize_path_string(normalization_buffer, expansion_buffer, type);
  if (result != LIBD_PF_FS_OK) {
    return result;
  }

  size_t length = strlen(normalization_buffer);
  if (length >= LIBD_PF_FS_PATH_MAX) {
    return LIBD_PF_FS_PATH_TOO_LONG;
  }

  out_path->length = length;
  strcpy(out_path->path, normalization_buffer);

  return LIBD_PF_FS_OK;
}

result_e
_expand_path_env_variables(char dest[WORK_BUFFER_MAX],
                           const char* path,
                           env_getter_f env_getter)
{
  // user declared no environment variables in this path
  if (env_getter == NULL) {
    strcpy(dest, path);
    return LIBD_PF_FS_OK;
  }

  if (path == NULL) {
    return LIBD_PF_FS_NULL_PARAMETER;
  }
  if (*path == '\0') {
    return LIBD_PF_FS_EMPTY_PATH;
  }
  if (strlen(path) >= WORK_BUFFER_MAX) {  // eq because of room for '\0'
    return LIBD_PF_FS_PATH_TOO_LONG;
  }

  size_t num_chars_copied = 0;
  uint8_t infinite_loop_protection = 0;
  uint8_t loop_max = 55;

  // pointers to bound environment variables
  char* env_start = NULL;
  char* env_end = NULL;

  // buffers where inspection and building of new paths happen
  char inspect_buffer[WORK_BUFFER_MAX];
  char build_buffer[WORK_BUFFER_MAX];
  char* inspect = inspect_buffer;
  char* build = build_buffer;
  char* temp;  // used for switching

  // return value from the getenv call
  const char* env_variable_expansion = NULL;

  // load the inspect buffer for the first time.
  strcpy(inspect, path);

  // loop until all environment variables are expanded;
  while (true) {
    if (infinite_loop_protection > loop_max) {
      return LIBD_PF_FS_TOO_MANY_ENV_EXPANSIONS;
    }
    infinite_loop_protection += 1;

    // try to find start of an environment variable '$';
    env_start = _find_char_or_end(inspect, '$');
    // if not found, exit loop;
    if (*env_start == '\0') {
      break;
    }

    // find end of env variable '/' or '\0';
    env_end = env_start;
    env_end = _find_char_or_end(env_end, '/');
    if (env_end - env_start == 1) {  // "$/" or "$\0"
      return LIBD_PF_FS_INVALID_PATH;
    }

    // The inspect buffer is being mutated to cut out env var name.
    *env_start = '\0';  // clobbering the leading '$'
    *env_end = '\0';    // clobbering the trailing '/' or '\0'

    env_variable_expansion = env_getter(env_start + 1);
    if (env_variable_expansion == NULL) {
      return LIBD_PF_FS_PATH_ENV_VAR_NOT_FOUND;
    }

    // building a new path from the env variable expansion
    num_chars_copied = snprintf(build, WORK_BUFFER_MAX, "%s%s/%s", inspect,
                                env_variable_expansion, env_end + 1);
    // checking for truncation
    if (num_chars_copied >= WORK_BUFFER_MAX) {  // eq because of room for '\0'
      return LIBD_PF_FS_PATH_TOO_LONG;
    }

    // swap pointers for the next iteration.
    temp = build;
    build = inspect;
    inspect = temp;
  }

  // copy into dest;
  strcpy(dest, inspect);

  return LIBD_PF_FS_OK;
}

result_e
_normalize_path_string(char dest[WORK_BUFFER_MAX],
                       const char src[WORK_BUFFER_MAX],
                       path_type_e path_type)
{
  if (*src == '\0') {
    return LIBD_PF_FS_INVALID_PATH;
  }
  if (path_type >= LIBD_PF_FS_PATH_TYPE_E_COUNT) {
    return LIBD_PF_FS_INVALID_PATH_TYPE;
  }
  const char* current = src;
  const char* peek = current + 1;  // always 1+ ahead of current
  char* writer = dest;

  while (*peek != '\0') {

    if (!_is_valid_portable_path_char(*peek)) {
      return LIBD_PF_FS_INVALID_PATH;
    }

    // found a "../", jumping writer back a dir.
    if (*peek == '.' && *(peek - 1) == '.' && *(peek + 1) == '/') {
      if (writer <= dest + 1) {  // already at root
        // TODO: I think this should error
        continue;
      }
      *writer = '\0';
      writer = strrchr(dest, '/');
      peek += 2;
      current = peek - 1;
    }

    if (*peek == '.') {
      // semantically meaningless path naming
      if (*(peek - 1) == '-' || *(peek + 1) == '-' || *(peek - 1) == '_' ||
          *(peek + 1) == '_') {
        return LIBD_PF_FS_INVALID_PATH;
      }
      // name ends with a '.'
      if (isalnum(*(peek - 1)) && *(peek + 1) == '/') {
        return LIBD_PF_FS_INVALID_PATH;
      }
      // '/.*' malformation
      if (*(peek - 1) == '/' && !_is_valid_to_follow_period(*(peek + 1))) {
        return LIBD_PF_FS_INVALID_PATH;
      }
      if (isalnum(*(peek - 1)) && !isalnum(*(peek + 1))) {
        return LIBD_PF_FS_INVALID_PATH;
      }
      // a run of '.'
      if (*(peek - 1) == '.' && *(peek + 1) == '.') {
        return LIBD_PF_FS_INVALID_PATH;
      }
      // skipping "/./"
      if (*(peek - 1) == '/' && *(peek + 1) == '/') {
        peek += 2;
        current = peek - 1;
      }
    }

    // enforced convention for tidy naming.
    if (*(peek) == '-' || *(peek) == '_') {
      // '-' and '_' should be used as word separators only.
      if (!isalnum(*(peek - 1)) || !isalnum(*(peek + 1))) {
        return LIBD_PF_FS_INVALID_PATH;
      }
    }

    if (*peek == '/') {
      if (*(peek - 1) != '/') {
        memcpy(writer, current, peek - current);
        writer = writer + (peek - current);
        *writer = '\0';
      }
      current = peek;
    }
    peek += 1;
  }

  memcpy(writer, current, peek - current);
  writer = writer + (peek - current - 1);

  // Normalizing the end of the path.
  switch (path_type) {
  case LIBD_PF_FS_DIRETORY:
    if (*writer != '/') {
      *writer = '/';
      *(writer + 1) = '\0';
    }
    break;
  case LIBD_PF_FS_FILE:
    if (*writer == '/') {
      *writer = '\0';
    }
    break;
  default:
    return LIBD_PF_FS_INVALID_PATH_TYPE;
  }

  return LIBD_PF_FS_OK;
}

const char*
libd_platform_filesystem_path_string(libd_platform_filesystem_path_o* path)
{
  return path->path;
}

static inline char*
_find_char_or_end(char* s, char c)
{
  while (*s && *s != c) {
    s++;
  }
  return s;
}

static inline bool
_is_valid_portable_path_char(const char c)
{
  return isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/';
}

static inline bool
_is_valid_to_follow_period(const char c)
{
  return isalnum(c) || c == '.' || c == '/';
}
