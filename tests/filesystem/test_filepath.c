#include "../../include/libd/common.h"
#include "../../include/libd/filesystem.h"
#include "../../include/libd/testing.h"

#include <stddef.h>
#include <string.h>

TEST(filepath_normalize)
{
  struct {
    const char* name;
    const char* input_path;
    const char* expected_path;
    enum libd_result expected_result;
  } tcs[] = {
    {
      .name            = "null input path\0",
      .input_path      = NULL,
      .expected_result = libd_invalid_parameter,
    },
    {
      .name            = "empty input path\0",
      .input_path      = "\0",
      .expected_result = libd_invalid_parameter,
    },
    {
      .name            = "no changes\0",
      .input_path      = "/zero\0",
      .expected_path   = "/zero\0",
      .expected_result = libd_ok,
    },
    {
      .name            = "remove extra separators 1\0",
      .input_path      = "//zero\0",
      .expected_result = libd_ok,
      .expected_path   = "/zero\0",
    },
    {
      .name            = "remove extra separators 2\0",
      .input_path      = "zero//\0",
      .expected_result = libd_ok,
      .expected_path   = "zero/\0",
    },
    {
      .name            = "remove extra separators 3\0",
      .input_path      = "zero//zero\0",
      .expected_result = libd_ok,
      .expected_path   = "zero/zero\0",
    },
    {
      .name            = "remove extra separators 4\0",
      .input_path      = "/////zero////zero///zero\0",
      .expected_result = libd_ok,
      .expected_path   = "/zero/zero/zero\0",
    },
    {
      .name            = "remove self-refs 1\0",
      .input_path      = "./zero\0",
      .expected_result = libd_ok,
      .expected_path   = "zero\0",
    },
    {
      .name            = "remove self-refs 2\0",
      .input_path      = "zero/./zero\0",
      .expected_result = libd_ok,
      .expected_path   = "zero/zero\0",
    },
    {
      .name            = "remove self-refs 3\0",
      .input_path      = "zero/./././zero\0",
      .expected_result = libd_ok,
      .expected_path   = "zero/zero\0",
    },
    {
      .name            = "remove self-refs 4\0",
      .input_path      = "zero/zero/./\0",
      .expected_result = libd_ok,
      .expected_path   = "zero/zero/\0",
    },
    {
      .name            = "abs parent ref 1\0",
      .input_path      = "../zero\0",
      .expected_result = libd_invalid_path,
      .expected_path   = "\0",
    },
    {
      .name            = "abs parent ref 2\0",
      .input_path      = "/zero/../\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "abs parent ref 3\0",
      .input_path      = "/zero/zero/../../\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "abs parent ref 4\0",
      .input_path      = "/zero/../zero/../\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
  };

  char dest[256] = { 0 };
  const char* name;
  for (usize i = 0; i < ARR_LEN(tcs); i += 1) {
    name = tcs[i].name;

    ASSERT_EQ_U(
      libd_filesystem_filepath_normalize(dest, 256, tcs[i].input_path),
      tcs[i].expected_result,
      "name='%s', number=%zu\n",
      tcs[i].name,
      i);

    if (tcs[i].expected_result == libd_ok) {
      ASSERT_EQ_STR(
        dest,
        tcs[i].expected_path,
        "name='%s', number='%zu'\n",
        tcs[i].name,
        i);
    }
  }
}

enum libd_result
test_filepath_expand_env_getter(
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

TEST(filepath_expand)
{
  struct {
    const char* name;
    const char* input_path;
    const char* expected_string;
    enum libd_result expected_result;
  } tcs[] = {
    {
      .name            = "string 1\0",
      .input_path      = "zero/zero/$one\0",
      .expected_string = "/zero/zero/zero/\0",
      .expected_result = libd_ok,
    },
  };

  char result_string[128];
  const char* name;
  for (size_t i = 0; i < ARR_LEN(tcs); i += 1) {
    name = tcs[i].name;
  }
}
