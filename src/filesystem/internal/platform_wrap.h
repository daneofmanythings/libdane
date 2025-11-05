#ifndef FILESYSTEM_PATH_PLATFORM_WRAP_H
#define FILESYSTEM_PATH_PLATFORM_WRAP_H

#include <stdbool.h>

bool
platform_is_env_var(const char* val);

void
platform_env_var_mem_free(const char* env_var_str);

#endif  // FILESYSTEM_PATH_PLATFORM_WRAP_H
