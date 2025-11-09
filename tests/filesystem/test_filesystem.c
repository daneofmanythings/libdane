#include "../../include/libd/common.h"
#include "../../include/libd/filesystem.h"
#include "../../include/libd/testing.h"

#include <string.h>

enum libd_result
filepath_env_getter(
  char* out_val,
  const char* key)
{
  if (strcmp(key, "one\0") == 0) {
    strcpy(out_val, "zero\0");
    return libd_ok;
  }
  if (strcmp(key, "two\0") == 0) {
    strcpy(out_val, "$one/zero\0");
    return libd_ok;
  }
  if (strcmp(key, "three\0") == 0) {
    strcpy(out_val, "$two/$one\0");
    return libd_ok;
  }

  return libd_env_var_not_found;
}

TEST(filepath_init)
{
  struct {
    const char* name;
    const char* input_path;
    enum libd_filesystem_path_type input_type;

    enum libd_result expected_result;

    const char* (*input_env_getter)(const char*);
  } tcs[] = {
    {},
  };
}
