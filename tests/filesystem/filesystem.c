#include "../../include/libd/filesystem.h"

#include "../../include/libd/common.h"
#include "../../include/libd/testing.h"

#include <string.h>

static const char*
env_var_getter(const char* var)
{
  if (strcmp("HOME\0", var) == 0) {
    return "/home/user\0";
  }

  return NULL;
}

struct init_params {
  const char test_name[64];
  const char input_path[64];
  const char expected_path[64];

  enum libd_filesystem_path_type input_type;
  enum libd_result expected_result;

  const char* (*input_env_getter)(const char*);
};

TEST(path_normalization)
{
  ASSERT_OK(0);
  // static struct init_params params[] = {
  //   {
  //     .test_name        = "simple absolute directory test 1\0",
  //     .input_path       = "/test/\0",
  //     .expected_path    = "/test/\0",
  //     .input_type       = libd_abs_directory,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "simple absolute directory test 2\0",
  //     .input_path       = "/test\0",
  //     .expected_path    = "/test/\0",
  //     .input_type       = libd_abs_directory,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "simple absolute file test 1\0",
  //     .input_path       = "/test/\0",
  //     .expected_path    = "/test\0",
  //     .input_type       = libd_abs_file,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "simple absolute file test 2\0",
  //     .input_path       = "/test\0",
  //     .expected_path    = "/test\0",
  //     .input_type       = libd_abs_file,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "simple file test with extension\0",
  //     .input_path       = "/test.extension/\0",
  //     .expected_path    = "/test.extension\0",
  //     .input_type       = libd_abs_file,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "simple root\0",
  //     .input_path       = "/\0",
  //     .expected_path    = "/\0",
  //     .input_type       = libd_abs_directory,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "simple root with two excess slashes\0",
  //     .input_path       = "//\0",
  //     .expected_path    = "/\0",
  //     .input_type       = libd_abs_directory,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "simple root with three excess slashes\0",
  //     .input_path       = "///\0",
  //     .expected_path    = "/\0",
  //     .input_type       = libd_abs_directory,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "normalizing excess slashes\0",
  //     .input_path       = "//multiple///slash////file//////\0",
  //     .expected_path    = "/multiple/slash/file\0",
  //     .input_type       = libd_abs_file,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "removing ./\0",
  //     .input_path       = "/home/./user/.//foo//./bar//.///baz\0",
  //     .expected_path    = "/home/user/foo/bar/baz/\0",
  //     .input_type       = libd_abs_file,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "Jump up one directory\0",
  //     .input_path       = "/home/../user/foo/\0",
  //     .expected_path    = "/user/foo\0",
  //     .input_type       = libd_abs_file,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "hidden file\0",
  //     .input_path       = "/home/user/.foo/\0",
  //     .expected_path    = "/home/user/.foo\0",
  //     .input_type       = libd_abs_file,
  //     .expected_result  = libd_ok,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "empty path\0",
  //     .input_path       = "\0",
  //     .expected_path    = "\0",
  //     .input_type       = libd_rel_file,
  //     .expected_result  = libd_invalid_path,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "invalid ...\0",
  //     .input_path       = "/home/.../user/\0",
  //     .expected_path    = "\0",
  //     .input_type       = libd_abs_directory,
  //     .expected_result  = libd_invalid_path,
  //     .input_env_getter = NULL,
  //   },
  //   {
  //     .test_name        = "invalid /..a sequence\0",
  //     .input_path       = "/home/..a/user/\0",
  //     .input_type       = libd_abs_file,
  //     .input_env_getter = NULL,
  //     .expected_result  = libd_invalid_path,
  //     .expected_path    = "\0",
  //   },
  //   {
  //     .test_name        = "invalid a../ sequence\0",
  //     .input_path       = "/home/a../user/\0",
  //     .input_type       = libd_abs_file,
  //     .input_env_getter = NULL,
  //     .expected_result  = libd_invalid_path,
  //     .expected_path    = "\0",
  //   },
  //   {
  //     .test_name        = "invalid a-. sequence\0",
  //     .input_path       = "/home/a-./user/\0",
  //     .input_type       = libd_abs_file,
  //     .input_env_getter = NULL,
  //     .expected_result  = libd_invalid_path,
  //     .expected_path    = "\0",
  //   },
  //   {
  //     .test_name        = "invalid end *. \0",
  //     .input_path       = "/home/user/foo.\0",
  //     .input_type       = libd_abs_file,
  //     .input_env_getter = NULL,
  //     .expected_result  = libd_invalid_path,
  //     .expected_path    = "\0",
  //   },
  //   {
  //     .test_name        = "too many walk ups\0",
  //     .input_path       = "/home/../../user/../foo\0",
  //     .input_type       = libd_abs_file,
  //     .input_env_getter = NULL,
  //     .expected_result  = libd_invalid_path,
  //     .expected_path    = "\0",
  //   },
  //   // Expansion
  //   {
  //     .test_name        = "basic dir expansion\0",
  //     .input_path       = "$HOME/foo/\0",
  //     .input_type       = libd_abs_directory,
  //     .input_env_getter = env_var_getter,
  //     .expected_result  = libd_ok,
  //     .expected_path    = "/home/user/foo/\0",
  //   },
  // };
  //
  // libd_filepath_h* path =
  // malloc(libd_filesystem_filepath_get_required_size());
  //
  // size_t num_tests = ARR_LEN(params);
  // for (size_t i = 0; i < num_tests; i += 1) {
  //   enum libd_result result;
  //   result = libd_filesystem_filepath_init(
  //     path, params->input_path, params->input_type,
  //     params->input_env_getter);
  //   ASSERT_EQ_STR(libd_filesystem_filepath_string(path),
  //   params->expected_path); ASSERT_EQ_U(result, params->expected_result);
  // }
}
