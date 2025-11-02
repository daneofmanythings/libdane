/**
 * @file filesystem.h
 * @brief A set of platform agnostic utilities for working with the filesystem.
 */

#include "../../../include/types.h"
#include "../../../platform/include/libdane/platform/filesystem.h"

/**
 * @brief Opaque type for libd_filesystem_path_s. Is used for path operations
 * with a safe, managed abstraction.
 */
typedef struct libd_filesystem_path_s libd_filesystem_path_o;

//==============================================================================
// Path API
//==============================================================================

/**
 * @brief Initializes a path_s.
 * @param out_filepath Out parameter for the created path.
 * @param path_string Path to initialize from.
 * @return RESULT_OK on success, non-zero otherwise.
 */
enum libd_result
libd_filesystem_path_create(
  libd_filesystem_path_o** out_path,
  const char* path_string);

//
enum libd_result
libd_filesystem_expand_env_vars(
  char dest[PATH_MAX],
  const char* path);
enum libd_result
write_new_file(
  const char* path,
  const char* body);
enum libd_result
write_to_scratch(
  const char* path,
  const char* body);
enum libd_result
mkdir_from_absolute_filepath(const char* filepath);
