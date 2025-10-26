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
  char path[PATH_BUFFER_MAX];
};

typedef libd_platform_filesystem_result_e result_e;
typedef libd_platform_filesystem_path_o path_o;
typedef libd_platform_filetype_path_type_e path_type_e;
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

libd_platform_filesystem_result_e
_rewind_one_directory(
  char* writer,
  const char* dest,
  const char* current,
  const char* peek);

static inline bool
_is_path_absolute(libd_platform_filetype_path_type_e path_type);

static inline bool
_is_path_directory(libd_platform_filetype_path_type_e path_type);

static inline char*
_find_char_or_end(
  char* s,
  char c);

result_e
libd_platform_filesystem_path_init(
  path_o* out_path,
  const char* raw_path,
  size_t raw_path_length_bytes,
  path_type_e type,
  env_getter_f env_getter)
{
  if (out_path == NULL || raw_path == NULL) {
    return LIBD_PF_FS_NULL_PARAMETER;
  }
  out_path->path[0] = '\0';

  char expansion_buffer[WORK_BUFFER_MAX];

  result_e result =
    _expand_path_env_variables(expansion_buffer, raw_path, env_getter);
  if (result != LIBD_PF_FS_OK) {
    return result;
  }

  char normalization_buffer[WORK_BUFFER_MAX];
  result = _normalize_path_string(normalization_buffer, expansion_buffer, type);
  if (result != LIBD_PF_FS_OK) {
    return result;
  }

  size_t length = strlen(normalization_buffer);
  if (length >= LIBD_PF_FS_PATH_MAX) {
    return LIBD_PF_FS_PATH_TOO_LONG;
  }

  out_path->length = length;
  strcpy(out_path->path, normalization_buffer);

  return LIBD_PF_FS_OK;
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
    return LIBD_PF_FS_OK;
  }

  if (path == NULL) {
    return LIBD_PF_FS_NULL_PARAMETER;
  }
  if (*path == '\0') {
    return LIBD_PF_FS_EMPTY_PATH;
  }
  if (strlen(path) >= WORK_BUFFER_MAX) {  // eq because of room for '\0'
    return LIBD_PF_FS_PATH_TOO_LONG;
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
      return LIBD_PF_FS_TOO_MANY_ENV_EXPANSIONS;
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
      return LIBD_PF_FS_INVALID_PATH;
    }

    // The inspect buffer is being mutated to cut out env var name.
    *env_start = '\0';  // clobbering the leading '$'
    *env_end   = '\0';  // clobbering the trailing '/' or '\0'

    env_variable_expansion = env_getter(env_start + 1);
    if (env_variable_expansion == NULL) {
      return LIBD_PF_FS_PATH_ENV_VAR_NOT_FOUND;
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
      return LIBD_PF_FS_PATH_TOO_LONG;
    }

    // swap pointers for the next iteration.
    temp    = build;
    build   = inspect;
    inspect = temp;
  }

  // copy into dest;
  strcpy(dest, inspect);

  return LIBD_PF_FS_OK;
}

#define SCANNER_PARAM   scanner
#define SCAN            (*(SCANNER_PARAM.peek))
#define JUST_SCANNED(c) ((*(SCANNER_PARAM.peek - 1)) == (c))
#define NOW_SCANNING(c) ((*SCANNER_PARAM.peek) == (c))
#define WILL_SCAN(c)    ((*(SCANNER_PARAM.peek + 1)) == (c))
typedef struct {
  const char* current;
  const char* peek;
} path_scanner;
void
_path_scanner_init(
  path_scanner* out_scanner,
  const char* src);
void
_path_scanner_jump(
  path_scanner* scanner,
  uintptr_t val);
void
_path_scanner_jump_sync(
  path_scanner* scanner,
  uintptr_t val);

static inline char
_scan(
  path_scanner* scanner,
  size_t offset);
static inline bool
_scan_is(
  path_scanner* scanner,
  int64_t offset,
  char c);

typedef struct {
  uint16_t write_width;
  bool is_directory;
} write_record;
typedef struct {
  char* head;
  char* dest;
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
void
_path_writer_write_from_scanner(
  path_writer* writer,
  path_scanner* scanner,
  bool is_dir_or_file);
void
_path_writer_write_parent_jump_from_scanner(
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
  path_scanner* scanner);

result_e
_normalize_path_string(
  char dest[WORK_BUFFER_MAX],
  const char src[WORK_BUFFER_MAX],
  path_type_e path_type)
{
  if (*src == '\0') {
    return LIBD_PF_FS_INVALID_PATH;
  }

  path_scanner s = {};
  _path_scanner_init(&s, src);
  path_writer w = {};
  _path_writer_init(&w, dest);

  _path_scanner_writer_normalize_path_beginning(&w, &s);

  libd_platform_filesystem_result_e result;
  while (!_scan_is(&s, 0, '\0')) {
    // Enforcing strict character policy
    if (!_is_valid_portable_path_char(*s.peek)) {
      return LIBD_PF_FS_INVALID_PATH;
    }

    // peeking a '.' is a hotspot.
    if (_scan_is(&s, 0, '.')) {
      // found a dotdot
      if (_scan_is(&s, -1, '.') && _scan_is(&s, 1, '/')) {
        if (_is_path_absolute(path_type)) {  // this always attemps a rewind
          if (_path_scanner_writer_rewind_directory(&s, &w) != 0) {
            return LIBD_PF_FS_INVALID_PATH;  // fails if out of bounds attempted
          }
        } else {  // relative branch
          if (_path_writer_previous_is_dir(&w)) {
            // cannot fail because parent is a directory
            _path_scanner_writer_rewind_directory(&s, &w);
          } else {  // write out the dotdot
            _path_writer_write_parent_jump_from_scanner(&w, &s);
            continue;
          }
        }
      }
      // semantically meaningless path naming. enforced convention.
      if (
        JUST_SCANNED('-') || WILL_SCAN('-') || JUST_SCANNED('_') ||
        WILL_SCAN('_')) {
        return LIBD_PF_FS_INVALID_PATH;
      }
      // name ends with a '.'
      if (isalnum(SCAN) && WILL_SCAN('/')) {
        return LIBD_PF_FS_INVALID_PATH;
      }
      // '/.*' malformation
      if (JUST_SCANNED('/') && WILL_SCAN('.') && *(peek + 2) != '/') {
        return LIBD_PF_FS_INVALID_PATH;
      }
      // '<alnum>.*' malformation
      if (isalnum(*(peek - 1)) && !isalnum(*(peek + 1))) {
        return LIBD_PF_FS_INVALID_PATH;
      }
      // a run of '.'
      if (*(peek - 1) == '.' && *(peek + 1) == '.') {
        return LIBD_PF_FS_INVALID_PATH;
      }
      // skipping "/./"
      if (*(peek - 1) == '/' && *(peek + 1) == '/') {
        _path_scanner_jump_sync(&s, 1);
      }
    }

    // enforced convention for tidy naming.
    if (NOW_SCANNING('-') || NOW_SCANNING('_')) {
      // '-' and '_' should be used as word separators only.
      if (!isalnum(*(peek - 1)) || !isalnum(*(peek + 1))) {
        return LIBD_PF_FS_INVALID_PATH;
      }
    }

    // potential write point has been hit. Skips past multiple '/'
    if (*peek == '/') {
      if (*(peek - 1) != '/') {
        _path_writer_write_from_scanner(&w, &s, true);
      }
      s.current = s.peek;
    }
    s.peek += 1;
  }

  // write out the remaining path segment.
  _path_writer_write_from_scanner(&w, &s, true);

  _path_writer_normalize_path_ending(&w);

  return LIBD_PF_FS_OK;
}

const char*
libd_platform_filesystem_path_string(libd_platform_filesystem_path_o* path)
{
  return path->path;
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

libd_platform_filesystem_result_e
_rewind_one_directory(
  char* writer,
  const char* dest,
  const char* current,
  const char* peek)
{
  if (writer == dest || (*dest == '/' && writer - dest == 1)) {
    return LIBD_PF_FS_PATH_OUT_OF_BOUNDS;
  }

  while (writer != dest && *writer != '/') {
    writer -= 1;
  }

  peek += 2;
  current = peek - 1;

  return LIBD_PF_FS_OK;
}

static inline bool
_is_path_absolute(libd_platform_filetype_path_type_e path_type)
{
  return (path_type & LIBD_PF_FS_IS_ABS) == 1;
}

static inline bool
_is_path_directory(libd_platform_filetype_path_type_e path_type)
{
  return (path_type & LIBD_PF_FS_IS_DIR) == 1;
}

void
_path_writer_init(
  path_writer* out_writer,
  char* dest)
{
  out_writer->head                          = dest;
  out_writer->dest                          = dest;
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
}

bool
_path_writer_previous_is_dir(path_writer* writer)
{
  if (writer->write_records_current_index == 0) {
    return false;
  }

  if (writer->write_records[writer->write_records_current_index].is_directory) {
    return true;
  }

  return false;
}

void
_path_writer_write_from_scanner(
  path_writer* writer,
  path_scanner* scanner,
  bool is_dir_or_file)
{
  uintptr_t length = scanner->peek - scanner->current;

  memcpy(writer->head, scanner->current, length);

  // updating writer state;
  writer->head += length;
  writer->write_records[writer->write_records_current_index].write_width =
    length;
  writer->write_records[writer->write_records_current_index].is_directory =
    is_dir_or_file;
  writer->write_records_current_index += 1;
}

void
_path_writer_write_parent_jump_from_scanner(
  path_writer* writer,
  path_scanner* scanner)
{
  peek += 1;
  _path_writer_write_from_scanner(writer, scanner, false);
  _path_scanner_jump_sync(scanner, 1);
}

void
_path_scanner_writer_normalize_path_beginning(
  path_writer* writer,
  path_scanner* scanner)
{
  // Normalizing the beginning of the path.
  if (_is_path_absolute(path_type) && src[0] != '/') {
    // bespoke operation for this condition.
    *writer->head = '/';
    writer->head += 1;
    writer->dest += 1;
  } else if (!_is_path_absolute(path_type) && src[0] == '/') {
    // or advance current and peek if relative with a '/'
    _path_scanner_jump_sync(&scanner, 1);
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
  out_scanner->current = src;
  out_scanner_.peek    = current + 1;  // always 1+ ahead of current
}
void
_path_scanner_jump(
  path_scanner* scanner,
  uintptr_t val)
{
  //
}

void
_path_scanner_jump_sync(
  path_scanner* scanner,
  uintptr_t val)
{
  scanner->peek += val;
  scanner->current = peek - 1;
}

int
_path_scanner_writer_rewind_directory(
  path_scanner* scanner,
  path_writer* writer)
{
  // rewind the writer.
  if (_path_writer_rewind(&writer) != 0) {  // will fail if out of bounds
    return return -1;
  }
  // align the scanner past the '../'
  _path_scanner_jump_sync(&scanner, 2);
}

static inline char
_scan(
  path_scanner* scanner,
  size_t offset)
{
  return *(scanner->peek + offset);
}
static inline bool
_scan_is(
  path_scanner* scanner,
  int64_t offset,
  char c)
{
  return _scan(scanner, offset) == c;
}
