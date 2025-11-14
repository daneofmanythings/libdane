#include "../../include/libd/common.h"
#include "../../include/libd/filesystem.h"
#include "../../include/libd/platform/filesystem.h"
#include "./internal/platform_wrap.h"

#include <stdbool.h>

static bool
_found_self_ref(
  const char* path,
  usize scan_pos);

static bool
_found_parent_ref(
  const char* path,
  usize scan_pos);

enum libd_result
libd_filesystem_filepath_normalize(
  char* out_path,
  size_t out_len,
  const char* input_path)
{
  if (
    out_path == NULL || out_len == 0 || input_path == NULL ||
    *input_path == NULL_TERMINATOR)
    return libd_invalid_parameter;

  size_t prefix_len = platform_filepath_prefix_len(input_path);

  size_t write_pos = 0;
  size_t scan_pos  = 0;

  while (scan_pos < prefix_len) {
    while (input_path[scan_pos + 1] == PATH_SEPARATOR) {
      scan_pos += 1;
    }
    out_path[write_pos++] = input_path[scan_pos++];
  }

  while (input_path[scan_pos] != NULL_TERMINATOR) {
    switch (input_path[scan_pos]) {
    case PATH_SEPARATOR:
      while (input_path[scan_pos + 1] == PATH_SEPARATOR) {
        scan_pos += 1;
      }
      break;
    case '.':
      if (_found_self_ref(input_path, scan_pos)) {
        scan_pos += 2;
        continue;
      } else if (_found_parent_ref(input_path, scan_pos)) {
        if (write_pos == prefix_len) {
          return libd_invalid_path;
        }
        write_pos -= 1;
        while (out_path[write_pos - 1] != PATH_SEPARATOR && write_pos > 0) {
          write_pos -= 1;
        }
        scan_pos += 3;
        continue;
      }
      break;
    default:
      break;
    }

    out_path[write_pos++] = input_path[scan_pos++];

    if (write_pos >= out_len) {
      return libd_err;
    }
  }

  out_path[write_pos] = NULL_TERMINATOR;

  return libd_ok;
}

static bool
_found_self_ref(
  const char* path,
  usize scan_pos)
{
  bool behind_satisfied = scan_pos == 0 || path[scan_pos - 1] == PATH_SEPARATOR;
  bool ahead_satisfied  = path[scan_pos + 1] == PATH_SEPARATOR;

  return behind_satisfied && ahead_satisfied;
}

static bool
_found_parent_ref(
  const char* path,
  usize scan_pos)
{
  if (path[scan_pos + 1] == NULL_TERMINATOR)
    return false;

  bool behind_satisfied = scan_pos == 0 || path[scan_pos - 1] == PATH_SEPARATOR;
  bool ahead_one_satisfied = path[scan_pos + 1] == '.';
  bool ahead_two_satisfied = path[scan_pos + 2] == PATH_SEPARATOR;

  return behind_satisfied && ahead_one_satisfied && ahead_two_satisfied;
}

enum libd_result
libd_filesystem_filepath_expand(
  char* out_path,
  size_t out_len,
  const char* input_path,
  libd_env_getter_f env_getter)
{
  //
}

enum libd_result
libd_filesystem_filepath_ancestor(
  char* out_path,
  size_t out_len,
  const char* start_path,
  uint16_t n);

// get extension

// strip extension

// has extension
