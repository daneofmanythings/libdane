#include "../../../include/libd/platform/filesystem.h"

#include <ctype.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

bool
libd_platform_filesystem_is_env_var(const char* key)
{
  if (key[0] != '$') {
    return false;
  }

  if (key[1] == '{') {
    size_t len = strlen(key);
    return key[len - 1] == '}';
  }

  return true;
}

#define _MAX_KEY_LEN 256
enum libd_result
libd_platform_filesystem_env_var_get(
  char* out_pointer,
  const char* key,
  enum libd_result (*env_getter)(
    char*,
    const char*))
{
  enum libd_result r = libd_ok;

  if (*key != '$') {
    return env_getter(out_pointer, key);
  }

  char temp[_MAX_KEY_LEN] = { 0 };
  size_t len              = strlen(key);
  if (len > _MAX_KEY_LEN) {
    return libd_invalid_path;  // FIX:
  }

  strncpy(temp, key, len);
  char* read_pos = &temp[1];

  if (temp[1] != '{') {
    return env_getter(out_pointer, read_pos);
  }

  read_pos += 1;
  if (temp[len - 1] != '}') {
    return libd_invalid_path;  // FIX:
  }
  temp[len - 1] = '\0';

  char* delim = strchr(read_pos, ':');

  if (delim == NULL) {
    return env_getter(out_pointer, read_pos);
  }

  *delim = '\0';
  r      = env_getter(out_pointer, &temp[2]);
  if (r != libd_ok) {
    read_pos = delim + 2;
    strcpy(out_pointer, read_pos);
    r = libd_ok;
  }

  return r;
}

static bool
is_component_value_delimeter_valid(
  const char* value,
  size_t i);
static bool
is_dashes_valid(
  const char* value,
  size_t i);

bool
libd_plaform_filesystem_is_explicit_component_value_valid(const char* value)
{
  if (*value == '.') {
    value += 1;
  }

  for (size_t i = 0; value[i] != '\0'; i += 1) {
    char c = value[i];
    switch (c) {
    case '.':
    case '-':
    case '_':
      if (!is_component_value_delimeter_valid(value, i)) {
        return false;
      }
      break;
    default:
      if (!isalnum(c)) {
        return false;
      }
    }
  }

  return true;
}

static bool
is_component_value_delimeter_valid(
  const char* value,
  size_t i)
{
  if (i == 0) {
    return false;
  }

  if (value[i + 1] == NULL_TERMINATOR) {
    return false;
  }

  if (!isalpha(value[i - 1] || !isalpha(value[i + 1]))) {
    return false;
  }

  return true;
}
