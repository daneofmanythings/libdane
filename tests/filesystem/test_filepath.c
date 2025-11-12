#include "../../include/libd/common.h"
#include "../../include/libd/filesystem.h"
#include "../../include/libd/testing.h"
#include "../../src/filesystem/filepath.h"

#include <stddef.h>
#include <string.h>

enum libd_result
test_filepath_env_getter(
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
    libd_filesystem_env_get_f input_env_get_f;
    enum libd_result expected_result;
  } tcs[] = {
    {
      "null input path\0",
      NULL,
      0,
      test_filepath_env_getter,
      libd_invalid_parameter,
    },
    {
      "empty input path\0",
      "\0",
      0,
      test_filepath_env_getter,
      libd_invalid_parameter,
    },
  };

  struct filepath fp = { 0 };
  const char* name;
  for (size_t i = 0; i < ARR_LEN(tcs); i += 1) {
    name = tcs[i].name;
    ASSERT_EQ_U(
      libd_filesystem_filepath_init(
        &fp, tcs[i].input_path, tcs[i].input_type, tcs[i].input_env_get_f),
      tcs[i].expected_result);
  }
}

TEST(filepath_string)
{
  struct {
    const char* name;
    const char* input_path;
    enum libd_filesystem_path_type input_type;
    libd_filesystem_env_get_f input_env_get_f;
    enum libd_result expected_result;
    const char* expected_string;
  } tcs[] = {
    {
      .name            = "string 1\0",
      .input_path      = "zero/zero/$one\0",
      .input_type      = libd_abs_directory,
      .input_env_get_f = test_filepath_env_getter,
      .expected_result = libd_ok,
      .expected_string = "/zero/zero/zero/\0",
    },
  };

  struct filepath fp = { 0 };
  char result_string[128];
  const char* name;
  for (size_t i = 0; i < ARR_LEN(tcs); i += 1) {
    name = tcs[i].name;
    ASSERT_OK(libd_filesystem_filepath_init(
      &fp, tcs[i].input_path, tcs[i].input_type, tcs[i].input_env_get_f));

    ASSERT_OK(libd_filesystem_filepath_string(&fp, result_string));
    ASSERT_EQ_STR(result_string, tcs[i].expected_string);
  }
}
