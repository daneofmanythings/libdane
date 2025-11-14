#include "./platform_wrap.h"

#include "../../../include/libd/platform/filesystem.h"

usize
platform_filepath_prefix_len(const char* path)
{
  return libd_platform_filesystem_filepath_prefix_len(path);
}
