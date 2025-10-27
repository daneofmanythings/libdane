#include "../../../include/libdane/platform/filesystem.h"

#include <ctype.h>
#include <linux/limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PATH_BUFFER_MAX (LIBD_PF_FS_PATH_MAX + 1)
#define WORK_BUFFER_MAX (2 * LIBD_PF_FS_PATH_MAX + 1)

struct libd_platform_filesystem_path_s {
  size_t length;
  char value[PATH_BUFFER_MAX];
};

typedef libd_platform_filesystem_result_e result_e;
typedef libd_platform_filesystem_path_o path_o;
typedef libd_platform_filesystem_path_type_e path_type_e;
typedef libd_platform_filesystem_env_getter_f env_getter_f;

result_e
_expand_path_env_variables(
  char dest[WORK_BUFFER_MAX],
  const char* path,
  env_getter_f env_getter);

result_e
_normalize_path_string(
  char dest_path[WORK_BUFFER_MAX],
  const char src_path[WORK_BUFFER_MAX],
  path_type_e path_type);

static inline bool
_is_valid_portable_path_char(const char c);

static inline bool
_is_path_absolute(path_type_e path_type);

static inline bool
_is_path_directory(path_type_e path_type);

static inline char*
_find_char_or_end(
  char* s,
  char c);

result_e
libd_platform_filesystem_path_init(
  path_o* out_path,
  const char* raw_path,
  path_type_e type,
  env_getter_f env_getter)
{
  if (out_path == NULL || raw_path == NULL) {
    return libd_pf_fs_null_parameter;
  }
  out_path->value[0] = '\0';

  char expansion_buffer[WORK_BUFFER_MAX];

  result_e result =
    _expand_path_env_variables(expansion_buffer, raw_path, env_getter);
  if (result != libd_pf_fs_ok) {
    return result;
  }

  char normalization_buffer[WORK_BUFFER_MAX];
  result = _normalize_path_string(normalization_buffer, expansion_buffer, type);
  if (result != libd_pf_fs_ok) {
    return result;
  }

  size_t length = strlen(normalization_buffer);
  if (length >= LIBD_PF_FS_PATH_MAX) {
    return libd_pf_fs_path_too_long;
  }

  out_path->length = length;
  strcpy(out_path->value, normalization_buffer);

  return libd_pf_fs_ok;
}

result_e
_expand_path_env_variables(
  char dest[WORK_BUFFER_MAX],
  const char* path,
  env_getter_f env_getter)
{
  // user declared no environment variables in this path
  if (env_getter == NULL) {
    strcpy(dest, path);
    return libd_pf_fs_ok;
  }

  if (path == NULL) {
    return libd_pf_fs_null_parameter;
  }
  if (*path == '\0') {
    return libd_pf_fs_empty_path;
  }
  if (strlen(path) >= WORK_BUFFER_MAX) {  // eq because of room for '\0'
    return libd_pf_fs_path_too_long;
  }

  size_t num_chars_copied          = 0;
  uint8_t infinite_loop_protection = 0;
  uint8_t loop_max                 = 55;

  // pointers to bound environment variables
  char* env_start = NULL;
  char* env_end   = NULL;

  // buffers where inspection and building of new paths happen
  char inspect_buffer[WORK_BUFFER_MAX];
  char build_buffer[WORK_BUFFER_MAX];
  char* inspect = inspect_buffer;
  char* build   = build_buffer;
  char* temp;  // used for switching

  // return value from the getenv call
  const char* env_variable_expansion = NULL;

  // load the inspect buffer for the first time.
  strcpy(inspect, path);

  // loop until all environment variables are expanded;
  while (true) {
    if (infinite_loop_protection > loop_max) {
      return libd_pf_fs_too_many_env_expansions;
    }
    infinite_loop_protection += 1;

    // try to find start of an environment variable '$';
    env_start = _find_char_or_end(inspect, '$');
    // if not found, exit loop;
    if (*env_start == '\0') {
      break;
    }

    // find end of env variable '/' or '\0';
    env_end = env_start;
    env_end = _find_char_or_end(env_end, '/');
    if (env_end - env_start == 1) {  // "$/" or "$\0"
      return libd_pf_fs_invalid_path;
    }

    // The inspect buffer is being mutated to cut out env var name.
    *env_start = '\0';  // clobbering the leading '$'
    *env_end   = '\0';  // clobbering the trailing '/' or '\0'

    env_variable_expansion = env_getter(env_start + 1);
    if (env_variable_expansion == NULL) {
      return libd_pf_fs_path_env_var_not_found;
    }

    // building a new path from the env variable expansion
    num_chars_copied = snprintf(
      build,
      WORK_BUFFER_MAX,
      "%s%s/%s",
      inspect,
      env_variable_expansion,
      env_end + 1);

    // checking for truncation
    if (num_chars_copied >= WORK_BUFFER_MAX) {  // eq because of room for '\0'
      return libd_pf_fs_path_too_long;
    }

    // swap pointers for the next iteration.
    temp    = build;
    build   = inspect;
    inspect = temp;
  }

  // copy into dest;
  strcpy(dest, inspect);

  return libd_pf_fs_ok;
}

#define SCANNER_PARAM scanner
// gives the char at the offset relative to peek
#define SCAN(peek_offset) (*(SCANNER_PARAM.scan_position + (peek_offset)))
typedef struct {
  const char* write_start;
  const char* scan_position;
} path_scanner;
void
_path_scanner_init(
  path_scanner* out_scanner,
  const char* src);
void
_path_scanner_jump_sync(
  path_scanner* scanner,
  uintptr_t val);

typedef struct {
  uint16_t write_width;
  bool is_directory;
} write_record;
typedef struct {
  char* head;
  uint16_t write_records_current_index;
  write_record write_records[PATH_BUFFER_MAX];
} path_writer;
void
_path_writer_init(
  path_writer* out_writer,
  char* dest);
int
_path_writer_rewind(path_writer* writer);
bool
_path_writer_previous_is_dir(path_writer* writer);
result_e
_path_writer_write_from_scanner(
  path_writer* writer,
  path_scanner* scanner,
  bool is_dir_or_file);
result_e
_path_writer_write_parent_reference_from_scanner(
  path_writer* writer,
  path_scanner* scanner);
void
_path_writer_normalize_directory_ending(path_writer* writer);
void
_path_writer_normalize_file_ending(path_writer* writer);
void
_path_writer_normalize_path_ending(
  path_writer* writer,
  path_type_e path_type);

int
_path_scanner_writer_rewind_directory(
  path_scanner* scanner,
  path_writer* writer);
void
_path_scanner_writer_normalize_path_beginning(
  path_writer* writer,
  path_scanner* scanner,
  path_type_e path_type);

result_e
_normalize_path_string(
  char dest_path[WORK_BUFFER_MAX],
  const char src_path[WORK_BUFFER_MAX],
  path_type_e path_type)
{
  if (!_is_valid_portable_path_char(src_path[0])) {
    return libd_pf_fs_invalid_path;
  }

  path_scanner scanner = { 0 };
  _path_scanner_init(&scanner, src_path);
  path_writer writer = { 0 };
  _path_writer_init(&writer, dest_path);

  _path_scanner_writer_normalize_path_beginning(&writer, &scanner, path_type);

  result_e result;
  while (SCAN(0) != '\0') {
    switch (SCAN(0)) {
    case '.':
      // found a parent reference '../'
      if (SCAN(-1) == '.' && SCAN(1) == '/') {
        if (_is_path_absolute(path_type)) {  // this always attemps a rewind
          if (_path_scanner_writer_rewind_directory(&scanner, &writer) != 0) {
            return libd_pf_fs_invalid_path;  // fails if out of bounds attempted
          }
        } else {  // relative branch
          if (_path_writer_previous_is_dir(&writer)) {
            // cannot fail because parent is a directory
            _path_scanner_writer_rewind_directory(&scanner, &writer);
          } else {  // write out the parent reference
            result = _path_writer_write_parent_reference_from_scanner(
              &writer, &scanner);
            if (result != libd_pf_fs_ok) {
              return result;
            }
          }
        }
        continue;
      }
      // name ends with a '.'
      if (isalnum(SCAN(-1)) && SCAN(1) == '/') {
        return libd_pf_fs_invalid_path;
      }
      // '/.*' malformation
      if (SCAN(-1) == '/' && SCAN(1) == '.' && SCAN(2) != '/') {
        return libd_pf_fs_invalid_path;
      }
      // '<alnum>.*' malformation
      if (isalnum(SCAN(-1)) && !isalnum(SCAN(1))) {
        return libd_pf_fs_invalid_path;
      }
      // a run of '.'
      if (SCAN(-1) == '.' && SCAN(1) == '.') {
        return libd_pf_fs_invalid_path;
      }
      // skipping "/./"
      if (SCAN(-1) == '/' && SCAN(1) == '/') {
        _path_scanner_jump_sync(&scanner, 2);
      }
      break;
    case '-':
    case '_':
      // '-' and '_' should be used as word separators only.
      if (!isalnum(SCAN(-1)) || !isalnum(SCAN(1))) {
        return libd_pf_fs_invalid_path;
      }
      break;
    case '/':
      if (SCAN(-1) != '/') {
        result = _path_writer_write_from_scanner(&writer, &scanner, true);
        if (result != libd_pf_fs_ok) {
          return result;
        }
      }
      scanner.write_start = scanner.scan_position;
      break;
    default:
      // Enforcing strict character policy
      if (!_is_valid_portable_path_char(SCAN(0))) {
        return libd_pf_fs_invalid_path;
      }
    }
    scanner.scan_position += 1;
  }

  // write out the remaining path segment.
  result = _path_writer_write_from_scanner(&writer, &scanner, true);
  if (result != libd_pf_fs_ok) {
    return result;
  }

  _path_writer_normalize_path_ending(&writer, path_type);

  return libd_pf_fs_ok;
}

const char*
libd_platform_filesystem_path_string(libd_platform_filesystem_path_o* path)
{
  return path->value;
}

static inline char*
_find_char_or_end(
  char* s,
  char c)
{
  while (*s && *s != c) {
    s++;
  }
  return s;
}

static inline bool
_is_valid_portable_path_char(const char c)
{
  return isalnum(c) || c == '.' || c == '-' || c == '_' || c == '/';
}

static inline bool
_is_path_absolute(libd_platform_filesystem_path_type_e path_type)
{
  return (path_type & LIBD_PF_FS_IS_ABS) == LIBD_PF_FS_IS_ABS;
}

static inline bool
_is_path_directory(libd_platform_filesystem_path_type_e path_type)
{
  return (path_type & LIBD_PF_FS_IS_DIR) == LIBD_PF_FS_IS_DIR;
}

void
_path_writer_init(
  path_writer* out_writer,
  char* dest)
{
  out_writer->head                          = dest;
  out_writer->write_records_current_index   = 0;
  out_writer->write_records[0].write_width  = 0;
  out_writer->write_records[0].is_directory = false;
}

int
_path_writer_rewind(path_writer* writer)
{
  if (writer->write_records_current_index == 0) {
    return -1;
  }
  // moving back the head
  writer->head -=
    writer->write_records[writer->write_records_current_index - 1].write_width;
  writer->write_records_current_index -= 1;

  return 0;
}

bool
_path_writer_previous_is_dir(path_writer* writer)
{
  if (writer->write_records_current_index == 0) {
    return false;
  }

  if (writer->write_records[writer->write_records_current_index - 1]
        .is_directory) {
    return true;
  }

  return false;
}

result_e
_path_writer_write_from_scanner(
  path_writer* writer,
  path_scanner* scanner,
  bool is_dir_or_file)
{
  if (writer->write_records_current_index >= PATH_BUFFER_MAX - 1) {
    return libd_pf_fs_path_too_long;
  }

  uintptr_t length = scanner->scan_position - scanner->write_start;

  memcpy(writer->head, scanner->write_start, length);

  // updating writer state;
  writer->head += length;
  *writer->head = '\0';
  writer->write_records[writer->write_records_current_index].write_width =
    length;
  writer->write_records[writer->write_records_current_index].is_directory =
    is_dir_or_file;
  writer->write_records_current_index += 1;

  return libd_pf_fs_ok;
}

result_e
_path_writer_write_parent_reference_from_scanner(
  path_writer* writer,
  path_scanner* scanner)
{
  scanner->scan_position += 1;

  result_e result = _path_writer_write_from_scanner(writer, scanner, false);
  if (result != libd_pf_fs_ok) {
    return result;
  }

  _path_scanner_jump_sync(scanner, 1);

  return libd_pf_fs_ok;
}

void
_path_scanner_writer_normalize_path_beginning(
  path_writer* writer,
  path_scanner* scanner,
  path_type_e path_type)
{
  // Normalizing the beginning of the path.
  if (_is_path_absolute(path_type) && *scanner->write_start != '/') {
    // bespoke operation for this condition.
    *writer->head = '/';
    writer->head += 1;
  } else if (!_is_path_absolute(path_type) && *scanner->write_start == '/') {
    // or advance current and peek if relative with a '/'
    _path_scanner_jump_sync(scanner, 1);
  }
}

void
_path_writer_normalize_directory_ending(path_writer* writer)
{
  if (*(writer->head - 1) != '/') {  // directory must end with '/'
    *(writer->head)     = '/';
    *(writer->head + 1) = '\0';
  }
}

void
_path_writer_normalize_file_ending(path_writer* writer)
{
  if (*(writer->head - 1) == '/') {  // file must not end with '/'
    *(writer->head - 1) = '\0';
  } else {
    *writer->head = '\0';
  }
}

void
_path_writer_normalize_path_ending(
  path_writer* writer,
  path_type_e path_type)
{
  if (_is_path_directory(path_type)) {
    _path_writer_normalize_directory_ending(writer);
  } else {
    _path_writer_normalize_file_ending(writer);
  }
}

void
_path_scanner_init(
  path_scanner* out_scanner,
  const char* src)
{
  out_scanner->write_start = src;
  out_scanner->scan_position =
    out_scanner->write_start + 1;  // always ahead of current
}

void
_path_scanner_jump_sync(
  path_scanner* scanner,
  uintptr_t val)
{
  scanner->scan_position += val;
  scanner->write_start = scanner->scan_position - 1;
}

int
_path_scanner_writer_rewind_directory(
  path_scanner* scanner,
  path_writer* writer)
{
  // rewind the writer.
  if (_path_writer_rewind(writer) != 0) {  // will fail if out of bounds
    return -1;
  }
  // align the scanner past the '../'
  _path_scanner_jump_sync(scanner, 2);

  return 0;
}
