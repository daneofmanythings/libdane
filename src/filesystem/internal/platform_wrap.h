#ifndef FILESYSTEM_PATH_PLATFORM_WRAP_H
#define FILESYSTEM_PATH_PLATFORM_WRAP_H

#include "../../../include/libd/filesystem.h"

#include <stdbool.h>

bool
platform_is_env_var(const char* key);

enum libd_result
platform_env_var_get(
  char* env_val,
  const char* env_key,
  libd_filesystem_env_get_f env_getter);

#endif  // FILESYSTEM_PATH_PLATFORM_WRAP_H
