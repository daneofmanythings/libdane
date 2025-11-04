#ifndef FILESYSTEM_PATH_PLATFORM_WRAP_H
#define FILESYSTEM_PATH_PLATFORM_WRAP_H

enum libd_result
platform_expand_path_env_variables(
  expansion_buffer,
  const char* raw_path,
  libd_filesystem_env_getter_f env_getter);

#endif  // FILESYSTEM_PATH_PLATFORM_WRAP_H
