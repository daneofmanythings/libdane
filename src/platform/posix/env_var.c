#include "../../../include/libd/platform/filesystem.h"

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
