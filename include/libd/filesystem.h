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

typedef enum libd_result (*libd_env_getter_f)(
  char*,
  const char*);

//==============================================================================
// Path API
//==============================================================================

/**
 * @brief Fills the out parameter with a normalized version of the input.
 * @param out Destination for the resulting path.
 * @param input Source path to normalize.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_filesystem_filepath_normalize(
  char* restrict out,
  size_t out_len,
  const char* restrict input);

/**
 * @brief Fills the out parameter with an environment expansion of the given
 * path using env_getter_f.
 * @param out Destination for the resulting path.
 * @param input Source path to expand.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_filesystem_filepath_expand(
  char* restrict out,
  size_t out_len,
  const char* restrict input,
  libd_env_getter_f env_getter_f);

/**
 * @brief Fills the out parameter with the result of joining two paths then
 * normalizing the result.
 * @param out Destination for the resulting path.
 * @param lhs Path to be joined on (left hand side).
 * @param rhs Path to join with (right hand side).
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_filesystem_filepath_join(
  char* restrict out,
  size_t out_len,
  const char* lhs,
  const char* rhs);

/**
 * @brief Fills the out parameter with the nth ancestor of the given path.
 * @param out Destination for the ancestor path.
 * @param input Source for the ancestor calculation.
 * @param n How many ancestor components up to walk.
 * return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_filesystem_filepath_ancestor(
  char* restrict out,
  size_t out_len,
  const char* restrict input,
  uint16_t n);

/**
 * @brief Checks if child is a subpath of parent. The comparison is
 * case-insensitive on Windows, case-sensitive on POSIX. A path is a subpath of
 * itself.
 * @param out Out parameter for the result of the comparison. Set to
 * true if child_path is a subpath of parent_path, false otherwise.
 * @param parent The potential parent/ancestor path.
 * @param child The potential child/descendant path to check.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_filesystem_filepath_is_subpath_of(
  bool* restrict out,
  size_t out_len,
  const char* restrict parent,
  const char* restrict child);

/**
 * @brief Checks if two paths are equal. The comparison is case-insensitive on
 * Windows and case-sensitive on POSIX.
 * @param out Out parameter for the result of the comparison. Set to
 * true if paths are equal, false otherwise.
 * @param lhs First path to compare.
 * @param rhs Second path to compare.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_filesystem_filepath_is_equal(
  bool* restrict out,
  size_t out_len,
  const char* restrict lhs,
  const char* restrict rhs);

/**
 * @brief Puts the filename into an out parameter if it exists. Will fail if
 * there is no file at the end.
 * @param out Destination to hold the filename.
 * @param input Source path.
 * @return LIBD_PF_FS_OK on success, non-zero otherwise.
 */
enum libd_result
libd_filesystem_filepath_filename(
  char* restrict out,
  size_t out_len,
  const char* restrict input);

/**
 * @brief
 */
enum libd_result
libd_filesystem_filepath_get_extention(
  char* restrict out,
  size_t out_len,
  const char* restrict input);

/**
 * @brief
 */
enum libd_result
libd_filesystem_filepath_strip_extention(
  char* restrict out,
  size_t out_len,
  const char* restrict input);

/**
 * @brief
 */
enum libd_result
libd_filesystem_filepath_has_extention(
  bool* restrict out,
  size_t out_len,
  const char* restrict input);

//==============================================================================
// Directory Management API
//==============================================================================

#endif  // LIBD_FILESYSTEM_H
