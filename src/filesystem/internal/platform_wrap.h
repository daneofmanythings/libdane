#ifndef FILESYSTEM_PATH_PLATFORM_WRAP_H
#define FILESYSTEM_PATH_PLATFORM_WRAP_H

#include "../../../include/libd/filesystem.h"

#include <stdbool.h>

const u8*
platform_filepath_end_of_prefix(const char* out_path);

usize
platform_char_byte_len(u8 byte);

u8
platform_write_char_to(
  u8* dest,
  const u8* src);

#endif  // FILESYSTEM_PATH_PLATFORM_WRAP_H
