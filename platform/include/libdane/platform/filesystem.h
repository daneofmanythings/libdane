/**
 * @file filesystem.h
 * @brief A set of utilities for non-portable filesystem actions.
 */

#ifndef LIBD_PLATFORM_FILESYSTEM_H
#define LIBD_PLATFORM_FILESYSTEM_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


//==============================================================================
// Result Codes
//==============================================================================

/**
 * @brief The result codes used by this library.
 */
typedef enum {
  LIBD_PF_FS_OK, /**< Operation successful */
  LIBD_PF_FS_NO_MEMORY,
  LIBD_PF_FS_NULL_PARAMETER,
  LIBD_PF_FS_PATH_INVALID, /**< The path provided is invalid */
  LIBD_PF_FS_PATH_TOO_LONG,
  LIBD_PF_FS_PATH_ENV_VAR_NOT_FOUND,
  LIBD_PF_FS_TOO_MANY_ENV_EXPANSIONS,
  LIBD_PLATFORM_FILESYSTEM_RESULT_E_COUNT, /**< The number of result codes */
} libd_platform_filesystem_result_e;

//==============================================================================
// Type definitions
//==============================================================================

/**
 * @brief Opaque handle to a path struct.
 */
typedef struct libd_platform_filesystem_path_s libd_platform_filesystem_path_o;

/**
 * @brief Used in the initialization of a path object to clarify the state of
 * the last component.
 */
typedef enum {
  LIBD_PF_FS_DIRETORY,
  LIBD_PF_FS_FILE,
} libd_platform_filetype_path_type_e;

//==============================================================================
// Path size definitions
//==============================================================================

#ifdef _WIN32
// Windows path limitations
// Note: Long paths (>260 characters) are not supported.
// The \\?\ prefix for long paths is not handled by this library.
#define LIBD_PF_FS_NAME_MAX 255
#define LIBD_PF_FS_PATH_MAX 260
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

//==============================================================================
// Path API
//==============================================================================


/**
 * @brief Normalizes and initialize a raw path into a path object.
 * @param out_path Out parameter for the path object.
 * @param raw_path The input path value.
 * @param raw_path_length_bytes Length of path_raw in bytes
 * @param type Indicate whether the path ends in a directory or a file.
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_platform_filesystem_result_e
libd_platform_filesystem_path_init(libd_platform_filesystem_path_o* out_path,
                                   const char* raw_path,
                                   size_t raw_path_length_bytes,
                                   libd_platform_filetype_path_type_e type);

/**
 * @brief Join two paths and initialize into the out parameter.
 * @param out_path Out parameter for the new path.
 * @param lhs_path Path to be joined on (left hand side).
 * @param rhs_path Path to join with (right hand side).
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_platform_filesystem_result_e
libd_platform_filesystem_path_join(
  libd_platform_filesystem_path_o* out_path,
  const libd_platform_filesystem_path_o* lhs_path,
  const libd_platform_filesystem_path_o* rhs_path);

/**
 * @brief Join two paths and mutate the lhs path with the result.
 * @param lhs_path Path to be appended to.
 * @param rhs_path Path to append.
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_platform_filesystem_result_e
libd_platform_filesystem_path_append(
  libd_platform_filesystem_path_o* lhs_path,
  const libd_platform_filesystem_path_o* rhs_path);

/**
 * @brief Gets the nth ancestor of the given path and initializes a new path
 * through the out parameter.
 * @param out_path Out parameter for the initialized ancestor path.
 * @param start_path Child path from which to get the ancestor.
 * @param n How many ancestors up to walk.
 * return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_platform_filesystem_result_e
libd_platform_filesystem_path_ancestor(
  libd_platform_filesystem_path_o* out_path,
  const libd_platform_filesystem_path_o* start_path,
  uint16_t n);

/**
 * @brief Checks if child_path is a subpath of parent_path. The comparison is
 * case-insensitive on Windows, case-sensitive on POSIX. A path is a subpath of
 * itself.
 * @param out_is_subpath Out parameter for the result of the comparison. Set to
 * true if child_path is a subpath of parent_path, false otherwise.
 * @param parent_path The potential parent/ancestor path.
 * @param child_path The potential child/descendant path to check.
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_platform_filesystem_result_e
libd_platform_filesystem_path_is_subpath_of(
  bool* out_is_subpath,
  const libd_platform_filesystem_path_o* parent_path,
  const libd_platform_filesystem_path_o* child_path);

/**
 * @brief Checks if two paths are equal. The comparison is case-insensitive on
 * Windows and case-sensitive on POSIX.
 * @param out_is_equal Out parameter for the result of the comparison. Set to
 * true if paths are equal, false otherwise.
 * @param path1 First path to compare.
 * @param path2 Second path to compare.
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_platform_filesystem_result_e
libd_platform_filesystem_path_is_equal(
  bool* out_is_equal,
  const libd_platform_filesystem_path_o* path1,
  const libd_platform_filesystem_path_o* path2);

/**
 * @brief Puts the filename into an out parameter if it exists. Will fail if
 * there is no file at the end.
 * @param out_path Path object to hold the filename.
 * @param filename_path Path object to extract the filename from.
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_platform_filesystem_result_e
libd_platform_filesystem_path_filename(
  libd_platform_filesystem_path_o* out_path,
  const libd_platform_filesystem_path_o* filename_path);

// get extension

// strip extension

// has extension


//==============================================================================
// Directory Management API
//==============================================================================

#endif  // LIBD_PLATFORM_FILESYSTEM_H
