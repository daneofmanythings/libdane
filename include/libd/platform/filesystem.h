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

/**
 * @brief Determines whether or not a given string format represents an
 * environment variable interpolation.
 * @param key The string to evaluate.
 */
bool
libd_platform_filesystem_is_env_var(const char* key);

enum libd_result
libd_platform_filesystem_env_var_get(
  char* out_pointer,
  const char* key,
  enum libd_result (*env_getter)(
    char*,
    const char*));

bool
libd_plaform_filesystem_is_explicit_component_value_valid(const char* value);

#endif  // LIBD_PLATFORM_FILESYSTEM_H
