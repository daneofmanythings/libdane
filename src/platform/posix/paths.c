#include "../../../include/libd/platform/filesystem.h"
#include "../../../include/libd/utils/encodings.h"

#include <stdbool.h>
#include <stddef.h>

const u8*
libd_platform_filesystem_filepath_end_of_prefix(const char* path)
{
  return (u8*)path;
}

u8
libd_platform_filesystem_filepath_write_char_encoding_to(
  u8* dest,
  const u8* src)
{
  return libd_utf8_write_char(dest, src);
}
