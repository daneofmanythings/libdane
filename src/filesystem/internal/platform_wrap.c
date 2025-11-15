#include "./platform_wrap.h"

#include "../../../include/libd/platform/filesystem.h"

const u8*
platform_filepath_end_of_prefix(const char* path)
{
  return libd_platform_filesystem_filepath_end_of_prefix(path);
}

u8
platform_write_char_to(
  u8* dest,
  const u8* src)
{
  return libd_platform_filesystem_filepath_write_char_encoding_to(dest, src);
}
