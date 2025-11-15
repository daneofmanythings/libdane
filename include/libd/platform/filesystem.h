/**
 * @file platform/filesystem.h
 * @brief A set of utilities for non-portable filesystem actions.
 */

#ifndef LIBD_PLATFORM_FILESYSTEM_H
#define LIBD_PLATFORM_FILESYSTEM_H

#include "../common.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//==============================================================================
// Path size definitions
//==============================================================================

#ifdef _WIN32
  // Windows path limitations
  // Note: Long paths (>260 characters) are not supported.
  // The \\?\ prefix for long paths is not handled by this library.
  #define LIBD_PF_FS_NAME_MAX        255
  #define LIBD_PF_FS_PATH_MAX        260
  #define LIBD_PF_FS_PATH_ALLOC_SIZE 272  // Conservatively sized for struct

#else  // POSIX
  #include <limits.h>

  #ifdef NAME_MAX
    #define LIBD_PF_FS_NAME_MAX NAME_MAX
  #else
    #define LIBD_PF_FS_NAME_MAX 255
  #endif

  #ifdef PATH_MAX
    #define LIBD_PF_FS_PATH_MAX PATH_MAX
  #else
    #define LIBD_PF_FS_PATH_MAX 4096
  #endif

  #define LIBD_PF_FS_PATH_ALLOC_SIZE 4112  // Conservatively sized for struct

#endif  // _WIN32

#ifdef _WIN32
  #define PATH_SEPARATOR_VALUE "\\\0"
  #define PATH_SEPARATOR       '\\'
#else
  #define PATH_SEPARATOR_VALUE "/\0"
  #define PATH_SEPARATOR       '/'
#endif

#define PATH_SEPARATOR_VALUE_LEN 2
#define PATH_SEPARATOR_LEN       1

//==============================================================================
// Path utilities
//==============================================================================

const u8*
libd_platform_filesystem_filepath_end_of_prefix(const char* out_path);

u8
libd_platform_filesystem_filepath_write_char_encoding_to(
  u8* dest,
  const u8* src);

#endif  // LIBD_PLATFORM_FILESYSTEM_H
