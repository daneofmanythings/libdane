#include "../../../include/libdane/platform/filesystem.h"
#include "./parsing.h"

#include <ctype.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_BUFFER_MAX (LIBD_PF_FS_PATH_MAX + 1)
#define WORK_BUFFER_MAX (2 * LIBD_PF_FS_PATH_MAX + 1)

struct libd_platform_filesystem_path_s {
  size_t length;
  char value[PATH_BUFFER_MAX];
};

typedef libd_platform_filesystem_result_e result_e;
typedef libd_platform_filesystem_path_o path_o;
typedef libd_platform_filesystem_path_type_e path_type_e;
typedef libd_platform_filesystem_env_getter_f env_getter_f;

static result_e
expand_path_env_variables(
  char dest[WORK_BUFFER_MAX],
  const char* path,
  env_getter_f env_getter);

static result_e
normalize_path_string(
  char dest_path[WORK_BUFFER_MAX],
  const char src_path[WORK_BUFFER_MAX],
  path_type_e path_type);

static inline bool
_is_valid_portable_path_char(const char c);

static inline bool
_is_path_absolute(path_type_e path_type);

static inline bool
_is_path_directory(path_type_e path_type);

static inline char*
_find_char_or_end(
  char* s,
  char c);

result_e
libd_platform_filesystem_path_init(
  path_o* out_path,
  const char* raw_path,
  path_type_e type,
  env_getter_f env_getter)
{
  if (out_path == NULL || raw_path == NULL) {
    return libd_pf_fs_null_parameter;
  }
  out_path->value[0] = '\0';

  char expansion_buffer[WORK_BUFFER_MAX];

  result_e result =
    expand_path_env_variables(expansion_buffer, raw_path, env_getter);
  if (result != libd_pf_fs_ok) {
    return result;
  }

  char normalization_buffer[WORK_BUFFER_MAX];
  result = normalize_path_string(normalization_buffer, expansion_buffer, type);
  if (result != libd_pf_fs_ok) {
    return result;
  }

  size_t length = strlen(normalization_buffer);
  if (length >= LIBD_PF_FS_PATH_MAX) {
    return libd_pf_fs_path_too_long;
  }

  out_path->length = length;
  strcpy(out_path->value, normalization_buffer);

  return libd_pf_fs_ok;
}

result_e
expand_path_env_variables(
  char dest[WORK_BUFFER_MAX],
  const char* path,
  env_getter_f env_getter)
{
  // user declared no environment variables in this path
  if (env_getter == NULL) {
    strcpy(dest, path);
    return libd_pf_fs_ok;
  }

  if (path == NULL) {
    return libd_pf_fs_null_parameter;
  }
  if (*path == '\0') {
    return libd_pf_fs_empty_path;
  }
  if (strlen(path) >= WORK_BUFFER_MAX) {  // eq because of room for '\0'
    return libd_pf_fs_path_too_long;
  }

  size_t num_chars_copied          = 0;
  uint8_t infinite_loop_protection = 0;
  uint8_t loop_max                 = 55;

  // pointers to bound environment variables
  char* env_start = NULL;
  char* env_end   = NULL;

  // buffers where inspection and building of new paths happen
  char inspect_buffer[WORK_BUFFER_MAX];
  char build_buffer[WORK_BUFFER_MAX];
  char* inspect = inspect_buffer;
  char* build   = build_buffer;
  char* temp;  // used for switching

  // return value from the getenv call
  const char* env_variable_expansion = NULL;

  // load the inspect buffer for the first time.
  strcpy(inspect, path);

  // loop until all environment variables are expanded;
  while (true) {
    if (infinite_loop_protection > loop_max) {
      return libd_pf_fs_too_many_env_expansions;
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
      return libd_pf_fs_invalid_path;
    }

    // The inspect buffer is being mutated to cut out env var name.
    *env_start = '\0';  // clobbering the leading '$'
    *env_end   = '\0';  // clobbering the trailing '/' or '\0'

    env_variable_expansion = env_getter(env_start + 1);
    if (env_variable_expansion == NULL) {
      return libd_pf_fs_path_env_var_not_found;
    }

    // building a new path from the env variable expansion
    num_chars_copied = snprintf(
      build,
      WORK_BUFFER_MAX,
      "%s%s/%s",
      inspect,
      env_variable_expansion,
      env_end + 1);

    // checking for truncation
    if (num_chars_copied >= WORK_BUFFER_MAX) {  // eq because of room for '\0'
      return libd_pf_fs_path_too_long;
    }

    // swap pointers for the next iteration.
    temp    = build;
    build   = inspect;
    inspect = temp;
  }

  // copy into dest;
  strcpy(dest, inspect);

  return libd_pf_fs_ok;
}

static result_e
normalize_path_string(
  char dest_path[WORK_BUFFER_MAX],
  const char src_path[WORK_BUFFER_MAX],
  path_type_e path_type)
{
  result_e result = libd_pf_fs_ok;

  enum libd_allocator_result tokenizer_result;
  struct libd_path_tokenizer* pt;
  tokenizer_result = libd_path_tokenizer_create(&pt, src_path, path_type);
  if (tokenizer_result != ok) {
    // TODO: make this more coherent
    result = libd_pf_fs_no_memory;
    goto exit;
  }

  tokenizer_result = libd_path_tokenizer_run(pt);
  if (tokenizer_result != ok) {
    // TODO: make this more coherent
    result = libd_pf_fs_path_too_long;  // or oob
    goto cleanup;
  }

  tokenizer_result = libd_path_tokenizer_evaluate(pt, dest_path);
  if (tokenizer_result != ok) {
    result = libd_pf_fs_invalid_path;
    goto cleanup;
  }

cleanup:
  libd_path_tokenizer_destroy(pt);
exit:
  return result;
}

const char*
libd_platform_filesystem_path_string(libd_platform_filesystem_path_o* path)
{
  return path->value;
}

static inline char*
_find_char_or_end(
  char* s,
  char c)
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
_is_path_absolute(libd_platform_filesystem_path_type_e path_type)
{
  return (path_type & LIBD_PF_FS_IS_ABS) == LIBD_PF_FS_IS_ABS;
}

static inline bool
_is_path_directory(libd_platform_filesystem_path_type_e path_type)
{
  return (path_type & LIBD_PF_FS_IS_DIR) == LIBD_PF_FS_IS_DIR;
}
