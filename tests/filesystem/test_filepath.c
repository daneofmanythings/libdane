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
      .input_path      = "/a\0",
      .expected_path   = "/a\0",
      .expected_result = libd_ok,
    },
    {
      .name            = "separators 1\0",
      .input_path      = "//a\0",
      .expected_result = libd_ok,
      .expected_path   = "/a\0",
    },
    {
      .name            = "separators 2\0",
      .input_path      = "a//\0",
      .expected_result = libd_ok,
      .expected_path   = "a/\0",
    },
    {
      .name            = "separators 3\0",
      .input_path      = "a//b\0",
      .expected_result = libd_ok,
      .expected_path   = "a/b\0",
    },
    {
      .name            = "separators 4\0",
      .input_path      = "/////a////b///c\0",
      .expected_result = libd_ok,
      .expected_path   = "/a/b/c\0",
    },
    {
      .name            = "relative 1\0",
      .input_path      = "./a\0",
      .expected_result = libd_ok,
      .expected_path   = "a\0",
    },
    {
      .name            = "relative 2\0",
      .input_path      = "a/./b\0",
      .expected_result = libd_ok,
      .expected_path   = "a/b\0",
    },
    {
      .name            = "relative 3\0",
      .input_path      = "a/./././b\0",
      .expected_result = libd_ok,
      .expected_path   = "a/b\0",
    },
    {
      .name            = "relative 4\0",
      .input_path      = "a/b/./\0",
      .expected_result = libd_ok,
      .expected_path   = "a/b/\0",
    },
    {
      .name            = "relative 5\0",
      .input_path      = ".\0",
      .expected_result = libd_ok,
      .expected_path   = "\0",
    },
    {
      .name            = "relative 6\0",
      .input_path      = "./\0",
      .expected_result = libd_ok,
      .expected_path   = "\0",
    },
    {
      .name            = "relative 7\0",
      .input_path      = "././\0",
      .expected_result = libd_ok,
      .expected_path   = "\0",
    },
    {
      .name            = "relative 8\0",
      .input_path      = "./../\0",
      .expected_result = libd_invalid_path,
    },
    {
      .name            = "parent ref 1\0",
      .input_path      = "../a\0",
      .expected_result = libd_invalid_path,
      .expected_path   = "\0",
    },
    {
      .name            = "parent ref 2\0",
      .input_path      = "/a/../\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "parent ref 3\0",
      .input_path      = "/a/b/../../\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "parent ref 4\0",
      .input_path      = "/a/../b/../\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "parent ref 5\0",
      .input_path      = "/a/..\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "parent ref 6\0",
      .input_path      = "/a/b/..\0",
      .expected_result = libd_ok,
      .expected_path   = "/a/\0",
    },
    {
      .name            = "parent ref 7\0",
      .input_path      = "/a/b/../\0",
      .expected_result = libd_ok,
      .expected_path   = "/a/\0",
    },
    {
      .name            = "parent ref 8\0",
      .input_path      = "/a/b/../.\0",
      .expected_result = libd_ok,
      .expected_path   = "/a/\0",
    },
    {
      .name            = "parent ref 9\0",
      .input_path      = "/../\0",
      .expected_result = libd_invalid_path,
    },
    {
      .name            = "parent ref 10\0",
      .input_path      = "../\0",
      .expected_result = libd_invalid_path,
    },
    {
      .name            = "parent ref 11\0",
      .input_path      = "../../\0",
      .expected_result = libd_invalid_path,
    },
    {
      .name            = "root 1\0",
      .input_path      = "/\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "root 2\0",
      .input_path      = "//\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "root 2\0",
      .input_path      = "///\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "root 3\0",
      .input_path      = "/./\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "root 4\0",
      .input_path      = "/./././\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "root 5\0",
      .input_path      = "/a/..\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    {
      .name            = "mixed 1\0",
      .input_path      = "a//b/./c/../d//\0",
      .expected_result = libd_ok,
      .expected_path   = "a/b/d/\0",
    },
    {
      .name            = "mixed 2\0",
      .input_path      = "a//./b///c/../\0",
      .expected_result = libd_ok,
      .expected_path   = "a/b/\0",
    },
    {
      .name            = "mixed 3\0",
      .input_path      = "///a//././b/../../c\0",
      .expected_result = libd_ok,
      .expected_path   = "/c\0",
    },
    {
      .name            = "mixed 4\0",
      .input_path      = "a/b/c///../../d/e/../\0",
      .expected_result = libd_ok,
      .expected_path   = "a/d/\0",
    },
    {
      .name            = "mixed 5\0",
      .input_path      = ".abc\0",
      .expected_result = libd_ok,
      .expected_path   = ".abc\0",
    },
    {
      .name            = "mixed 6\0",
      .input_path      = "./.abc\0",
      .expected_result = libd_ok,
      .expected_path   = ".abc\0",
    },
    {
      .name            = "mixed 7\0",
      .input_path      = "a/.b/c\0",
      .expected_result = libd_ok,
      .expected_path   = "a/.b/c\0",
    },
    {
      .name            = "mixed 8\0",
      .input_path      = "/a/.b/.c/..\0",
      .expected_result = libd_ok,
      .expected_path   = "/a/.b/\0",
    },
    {
      .name            = "mixed 9\0",
      .input_path      = ".///a//b\0",
      .expected_result = libd_ok,
      .expected_path   = "/a/b\0",
    },
    {
      .name            = "mixed 10\0",
      .input_path      = "a/.../b\0",
      .expected_result = libd_ok,
      .expected_path   = "a/.../b\0",
    },
    {
      .name            = "mixed 11\0",
      .input_path      = "/.../../a\0",
      .expected_result = libd_ok,
      .expected_path   = "/a\0",
    },
    {
      .name            = "mixed 12\0",
      .input_path      = "a/..hidden\0",
      .expected_result = libd_ok,
      .expected_path   = "a/..hidden\0",
    },
    {
      .name            = "mixed 13\0",
      .input_path      = "a/.hidden./b\0",
      .expected_result = libd_ok,
      .expected_path   = "a/.hidden./b\0",
    },
    {
      .name            = "mixed 14\0",
      .input_path      = "a/..../\0",
      .expected_result = libd_ok,
      .expected_path   = "a/..../\0",
    },
    {
      .name            = "mixed 15\0",
      .input_path      = "///a/////\0",
      .expected_result = libd_ok,
      .expected_path   = "/a/\0",
    },
    {
      .name            = "mixed 16\0",
      .input_path      = "a///./././b////c\0",
      .expected_result = libd_ok,
      .expected_path   = "a/b/c\0",
    },
    {
      .name            = "mixed 17\0",
      .input_path      = "a/b/c/d/e/f/../../../../../\0",
      .expected_result = libd_ok,
      .expected_path   = "a/\0",
    },
    {
      .name            = "mixed 19\0",
      .input_path      = "$HOME/a/./b\0",
      .expected_result = libd_ok,
      .expected_path   = "$HOME/a/b\0",
    },
    {
      .name            = "mixed 20\0",
      .input_path      = "~/a/..\0",
      .expected_result = libd_ok,
      .expected_path   = "~/\0",
    },
    {
      .name            = "mixed 21\0",
      .input_path      = "/$$/./..\0",
      .expected_result = libd_ok,
      .expected_path   = "/\0",
    },
    // utf8 support:
    // α/β/γ
    // /α/./β/../γ
    // καλος/./../μονο/
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
      .name            = "no changes\0",
      .input_path      = "zero/zero/\0",
      .expected_string = "zero/zero/\0",
      .expected_result = libd_ok,
    },
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
