/**
 * @file filesystem.h
 * @brief A set of platform agnostic utilities for working with the filesystem.
 */

#ifndef LIBD_FILESYSTEM_H
#define LIBD_FILESYSTEM_H

#include "common.h"

#include <stdbool.h>
#include <stddef.h>

//==============================================================================
// Filesystem types
//==============================================================================

/**
 * @brief Opaque type for libd_filesystem_path_s. Is used for path operations
 * with a safe, managed abstraction.
 */
typedef struct filepath libd_filepath_h;

/**
 * @brief Signature for a get env wrapper. Consumer implemented.
 * @param out Out parameter to the memory to store the val.
 * @param key The key for the environment variable.
 * @return result code for the operation.
 */
typedef enum libd_result (*libd_filesystem_env_get_f)(
  char* out,
  const char* key);

/**
 * @brief Possibly frees memory allocated by libd_filesystem_env_get_f.
 * @param var Pointer to the allocated memory.
 */
typedef void (*libd_filesystem_env_free_f)(const char* var);

/**
 * @brief Used in the initialization of a path object to clarify the type of
 * the path. The state is encoded into the path string during initialization.
 * @note The order of this enum should not change because it allows for bit
 * masking. Bit 0 is rel(0)/abs(1) and bit 1 is file(0)/dir(1).
 */
enum libd_filesystem_path_type {
  libd_rel_file = 0,       // 0b0000
  libd_abs_file = 1,       // 0b0001
  libd_rel_directory = 2,  // 0b0010
  libd_abs_directory = 3,  // 0b0011
  //
  libd_filesystem_path_type_count,
};

#define LIBD_FILEPATH_IS_ABS 1  // bit 0
#define LIBD_FILEPATH_IS_DIR 2  // bit 1

//==============================================================================
// Path API
//==============================================================================

/**
 * @brief Gets the allocation size needed for the opaque handle.
 * @return allocation size in bytes.
 */
size_t
libd_filesystem_filepath_get_required_size(void);

/**
 * @brief Normalizes and initialize a raw path into a path object.
 * @param out_path Out parameter for the path object.
 * @param raw_path The input path value.
 * @param type Indicate whether the path ends in a directory or a file.
 * @param env_getter The function used to retrieve environment variables.
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
enum libd_result
libd_filesystem_filepath_init(
  libd_filepath_h* out_path,
  const char* raw_path,
  enum libd_filesystem_path_type type,
  libd_filesystem_env_get_f env_getter);

/**
 * @brief Join two paths and initialize into the out parameter.
 * @param out_path Out parameter for the new path.
 * @param lhs_path Path to be joined on (left hand side).
 * @param rhs_path Path to join with (right hand side).
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_filepath_h
libd_filesystem_filepath_join(
  libd_filepath_h* out_path,
  const libd_filepath_h* lhs_path,
  const libd_filepath_h* rhs_path);

/**
 * @brief Join two paths and mutate the lhs path with the result.
 * @param lhs_path Path to be appended to.
 * @param rhs_path Path to append.
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_filepath_h
libd_filesystem_filepath_append(
  libd_filepath_h* lhs_path,
  const libd_filepath_h* rhs_path);

/**
 * @brief Gets the nth ancestor of the given path and initializes a new path
 * through the out parameter.
 * @param out_path Out parameter for the initialized ancestor path.
 * @param start_path Child path from which to get the ancestor.
 * @param n How many ancestors up to walk.
 * return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_filepath_h
libd_filesystem_filepath_ancestor(
  libd_filepath_h* out_path,
  const libd_filepath_h* start_path,
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
libd_filepath_h
libd_filesystem_filepath_is_subpath_of(
  bool* out_is_subpath,
  const libd_filepath_h* parent_path,
  const libd_filepath_h* child_path);

/**
 * @brief Checks if two paths are equal. The comparison is case-insensitive on
 * Windows and case-sensitive on POSIX.
 * @param out_is_equal Out parameter for the result of the comparison. Set to
 * true if paths are equal, false otherwise.
 * @param path1 First path to compare.
 * @param path2 Second path to compare.
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_filepath_h
libd_filesystem_filepath_is_equal(
  bool* out_is_equal,
  const libd_filepath_h* left_path,
  const libd_filepath_h* right_path);

/**
 * @brief Puts the filename into an out parameter if it exists. Will fail if
 * there is no file at the end.
 * @param out_path Path object to hold the filename.
 * @param filename_path Path object to extract the filename from.
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
libd_filepath_h
libd_filesystem_filepath_filename(
  libd_filepath_h* out_path,
  const libd_filepath_h* filename_path);

/**
 * @brief Returns a pointer to the internal path string.
 * @param path Path to examine
 * @return pointer to the internal path string.
 */
const char*
libd_filesystem_filepath_string(libd_filepath_h* path);

// get extension

// strip extension

// has extension

//==============================================================================
// Directory Management API
//==============================================================================

#endif  // LIBD_FILESYSTEM_H
