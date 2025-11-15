#include "../../include/libd/common.h"
#include "../../include/libd/filesystem.h"
#include "../../include/libd/platform/filesystem.h"
#include "./internal/platform_wrap.h"

#include <stdbool.h>

static bool
_found_relative_ref(
  const char* path,
  u8* scan_pos);

static bool
_found_parent_ref(
  const char* path,
  u8* scan_pos);

enum libd_result
libd_filesystem_filepath_normalize(
  char* out_path,
  usize out_len,
  const char* input_path)
{
  if (
    out_path == NULL || out_len == 0 || input_path == NULL ||
    *input_path == NULL_TERMINATOR)
    return libd_invalid_parameter;

  const u8* path_start = platform_filepath_end_of_prefix(input_path);

  u8* write_pos = (u8*)out_path;
  u8* scan_pos  = (u8*)input_path;

  u8 offset = 0;
  while (scan_pos < path_start) {
    offset = platform_write_char_to(write_pos, scan_pos);
    write_pos += offset;
    scan_pos += offset;
  }

  const u8* const write_path_start = write_pos;

  while (*scan_pos != NULL_TERMINATOR) {
    switch (*scan_pos) {
    case PATH_SEPARATOR:
      while (scan_pos[1] == PATH_SEPARATOR) {
        scan_pos += 1;
      }
      goto write_char;
    case DOT:
      if (_found_relative_ref(input_path, scan_pos)) {
        scan_pos += 2;
        continue;
      }
      if (_found_parent_ref(input_path, scan_pos)) {
        if (
          PTR_EQ(write_pos, write_path_start) ||
          PTR_EQ(write_pos - 1, write_path_start)) {
          return libd_invalid_path;
        }
        write_pos -= 1;

        while (write_pos[-1] != PATH_SEPARATOR && write_pos > path_start) {
          write_pos -= 1;
        }
        scan_pos += 3;
        continue;
      }
      goto write_char;
    default:
      goto write_char;
    }

  write_char:
    offset = platform_write_char_to(write_pos, scan_pos);
    write_pos += offset;
    scan_pos += offset;

    if (PTR_DIFF(write_pos, out_path) >= out_len) {
      return libd_err;
    }
  }

  *write_pos = NULL_TERMINATOR;

  return libd_ok;
}

static bool
_found_relative_ref(
  const char* path,
  u8* scan_pos)
{
  bool behind_satisfied =
    PTR_EQ(scan_pos, path) || scan_pos[-1] == PATH_SEPARATOR;
  bool ahead_satisfied =
    scan_pos[1] == PATH_SEPARATOR || scan_pos[1] == NULL_TERMINATOR;

  return behind_satisfied && ahead_satisfied;
}

static bool
_found_parent_ref(
  const char* path,
  u8* scan_pos)
{
  if (scan_pos[1] == NULL_TERMINATOR)
    return false;

  bool behind_satisfied =
    PTR_EQ(scan_pos, path) || scan_pos[-1] == PATH_SEPARATOR;
  bool ahead_one_satisfied = scan_pos[1] == DOT;
  bool ahead_two_satisfied =
    scan_pos[2] == PATH_SEPARATOR || scan_pos[2] == NULL_TERMINATOR;

  return behind_satisfied && ahead_one_satisfied && ahead_two_satisfied;
}

enum libd_result
libd_filesystem_filepath_expand(
  char* out_path,
  size_t out_len,
  const char* input_path,
  libd_env_getter_f env_getter)
{
  // setup a temp vector.
  // isolate and verify an environment variable for the platform.
  // fetch the value.
  // piece together the new string
  // repeat until done.
  // memmove to out_path.

  return libd_ok;
}

enum libd_result
libd_filesystem_filepath_ancestor(
  char* out_path,
  size_t out_len,
  const char* start_path,
  uint16_t n)
{
  //
}
