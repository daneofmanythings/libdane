#include "../../../include/libd/platform/filesystem.h"

#include <stdbool.h>
#include <stddef.h>

usize
libd_platform_filesystem_filepath_prefix_len(const char* path)
{
  usize result = 0;
  if (*path == PATH_SEPARATOR)
    result = 1;

  return result;
}
