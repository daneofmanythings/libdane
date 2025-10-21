#include "../../../include/libdane/platform/filesystem.h"
#include <linux/limits.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define PATH_BUFFER_MAX (LIBD_PF_FS_PATH_MAX + 1)
#define WORK_BUFFER_MAX (2 * PATH_BUFFER_MAX)

struct libd_platform_filesystem_path_s {
  size_t length;
  char path[PATH_BUFFER_MAX];
};

typedef libd_platform_filesystem_result_e result_e;
typedef libd_platform_filesystem_path_o path_o;
typedef libd_platform_filetype_path_type_e path_type_e;

result_e
_expand_path_env_variables(char dest[WORK_BUFFER_MAX], const char* path);
result_e
_normalize_path_string(char dest_path[WORK_BUFFER_MAX],
                       const char src_path[WORK_BUFFER_MAX],
                       path_type_e path_type);

static inline char*
_find_char_or_end(char* s, char c);

result_e
libd_platform_filesystem_path_init(path_o* out_path,
                                   const char* raw_path,
                                   size_t raw_path_length_bytes,
                                   path_type_e type)
{
  char expansion_buffer[WORK_BUFFER_MAX];
  result_e result = _expand_path_env_variables(expansion_buffer, raw_path);
  if (result != LIBD_PF_FS_OK) {
    return result;
  }

  char normalization_buffer[WORK_BUFFER_MAX];
  result = _normalize_path_string(normalization_buffer, expansion_buffer, type);

  out_path->length = strlen(normalization_buffer);
  // TODO: check path length here?

  strcpy(out_path->path, normalization_buffer);

  return LIBD_PF_FS_OK;
}

result_e
_expand_path_env_variables(char dest[WORK_BUFFER_MAX], const char* path)
{
  if (path == NULL) {
    return LIBD_PF_FS_NULL_PARAMETER;
  }
  if (strlen(path) >= LIBD_PF_FS_PATH_MAX) {
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

  // return value from the getenv call
  char* env_variable_expansion = NULL;

  // load the inspect buffer for the first time.
  strcpy(inspect_buffer, path);

  // loop until all environment variables are expanded;
  while (true) {
    if (infinite_loop_protection > loop_max) {
      return LIBD_PF_FS_TOO_MANY_ENV_EXPANSIONS;
    }
    infinite_loop_protection += 1;

    // try to find start of an environment variable '$';
    env_start = _find_char_or_end(inspect_buffer, '$');
    // if not found, exit loop;
    if (*env_start == '\0') {
      break;
    }

    // find end of env variable '/' or '\0';
    env_end = env_start;
    env_end = _find_char_or_end(env_end, '/');
    if (env_end - env_start == 1) {  // "$/" or "$\0"
      return LIBD_PF_FS_PATH_INVALID;
    }

    // The inspect buffer is being mutated to accomodate multiple manipulation
    // effects. It will be refreshed with the expanded value at the end.
    *env_start = '\0';  // clobbering the leading '$'
    *env_end = '\0';    // clobbering the trailing '/'

    env_variable_expansion = getenv(env_start + 1);
    if (env_variable_expansion == NULL) {
      return LIBD_PF_FS_PATH_ENV_VAR_NOT_FOUND;
    }

    // building a new path from the env variable expansion
    num_chars_copied =
      snprintf(build_buffer, WORK_BUFFER_MAX, "%s%s/%s", inspect_buffer,
               env_variable_expansion, env_end + 1);
    // checking for truncation
    if (num_chars_copied > WORK_BUFFER_MAX) {
      return LIBD_PF_FS_PATH_TOO_LONG;
    }

    // refresh the inspect buffer for the next iteration.
    strcpy(inspect_buffer, build_buffer);
  }

  // copy into dest;
  strcpy(dest, inspect_buffer);

  return LIBD_PF_FS_OK;
}

result_e
_normalize_path_string(char dest[WORK_BUFFER_MAX],
                       const char src[WORK_BUFFER_MAX],
                       path_type_e path_type)
{
  strcpy(dest, src);
  return LIBD_PF_FS_OK;
}

static inline char*
_find_char_or_end(char* s, char c)
{
  while (*s && *s != c) {
    s++;
  }
  return s;
}
