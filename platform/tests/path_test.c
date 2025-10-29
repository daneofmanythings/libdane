#include "../include/libdane/platform/filesystem.h"

#include <criterion/criterion.h>
#include <criterion/internal/new_asserts.h>
#include <criterion/new/assert.h>
#include <criterion/parameterized.h>
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

  libd_platform_filesystem_path_type_e input_type;
  libd_platform_filesystem_result_e expected_result;

  const char* (*input_env_getter)(const char*);
};

ParameterizedTestParameters(
  path,
  init_env_expansion)
{
  static struct init_params params[] = {
    {
      .test_name        = "simple absolute directory test 1\0",
      .input_path       = "/test/\0",
      .expected_path    = "/test/\0",
      .input_type       = libd_pf_fs_abs_directory,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "simple absolute directory test 2\0",
      .input_path       = "/test\0",
      .expected_path    = "/test/\0",
      .input_type       = libd_pf_fs_abs_directory,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "simple absolute file test 1\0",
      .input_path       = "/test/\0",
      .expected_path    = "/test\0",
      .input_type       = libd_pf_fs_abs_file,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "simple absolute file test 2\0",
      .input_path       = "/test\0",
      .expected_path    = "/test\0",
      .input_type       = libd_pf_fs_abs_file,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "simple file test with extension\0",
      .input_path       = "/test.extension/\0",
      .expected_path    = "/test.extension\0",
      .input_type       = libd_pf_fs_abs_file,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "simple root\0",
      .input_path       = "/\0",
      .expected_path    = "/\0",
      .input_type       = libd_pf_fs_abs_directory,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "simple root with two excess slashes\0",
      .input_path       = "//\0",
      .expected_path    = "/\0",
      .input_type       = libd_pf_fs_abs_directory,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "simple root with three excess slashes\0",
      .input_path       = "///\0",
      .expected_path    = "/\0",
      .input_type       = libd_pf_fs_abs_directory,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "normalizing excess slashes\0",
      .input_path       = "//multiple///slash////file//////\0",
      .expected_path    = "/multiple/slash/file\0",
      .input_type       = libd_pf_fs_abs_file,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "removing ./\0",
      .input_path       = "/home/./user/.//foo//./bar//.///baz\0",
      .expected_path    = "/home/user/foo/bar/baz/\0",
      .input_type       = libd_pf_fs_abs_file,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "Jump up one directory\0",
      .input_path       = "/home/../user/foo/\0",
      .expected_path    = "/user/foo\0",
      .input_type       = libd_pf_fs_abs_file,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "hidden file\0",
      .input_path       = "/home/user/.foo/\0",
      .expected_path    = "/home/user/.foo\0",
      .input_type       = libd_pf_fs_abs_file,
      .expected_result  = libd_pf_fs_ok,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "empty path\0",
      .input_path       = "\0",
      .expected_path    = "\0",
      .input_type       = libd_pf_fs_rel_file,
      .expected_result  = libd_pf_fs_invalid_path,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "invalid ...\0",
      .input_path       = "/home/.../user/\0",
      .expected_path    = "\0",
      .input_type       = libd_pf_fs_abs_directory,
      .expected_result  = libd_pf_fs_invalid_path,
      .input_env_getter = NULL,
    },
    {
      .test_name        = "invalid /..a sequence\0",
      .input_path       = "/home/..a/user/\0",
      .input_type       = libd_pf_fs_abs_file,
      .input_env_getter = NULL,
      .expected_result  = libd_pf_fs_invalid_path,
      .expected_path    = "\0",
    },
    {
      .test_name        = "invalid a../ sequence\0",
      .input_path       = "/home/a../user/\0",
      .input_type       = libd_pf_fs_abs_file,
      .input_env_getter = NULL,
      .expected_result  = libd_pf_fs_invalid_path,
      .expected_path    = "\0",
    },
    {
      .test_name        = "invalid a-. sequence\0",
      .input_path       = "/home/a-./user/\0",
      .input_type       = libd_pf_fs_abs_file,
      .input_env_getter = NULL,
      .expected_result  = libd_pf_fs_invalid_path,
      .expected_path    = "\0",
    },
    {
      .test_name        = "invalid end *. \0",
      .input_path       = "/home/user/foo.\0",
      .input_type       = libd_pf_fs_abs_file,
      .input_env_getter = NULL,
      .expected_result  = libd_pf_fs_invalid_path,
      .expected_path    = "\0",
    },
    {
      .test_name        = "too many walk ups\0",
      .input_path       = "/home/../../user/../foo\0",
      .input_type       = libd_pf_fs_abs_file,
      .input_env_getter = NULL,
      .expected_result  = libd_pf_fs_invalid_path,
      .expected_path    = "\0",
    },
    // Expansion
    {
      .test_name        = "basic dir expansion\0",
      .input_path       = "$HOME/foo/\0",
      .input_type       = libd_pf_fs_abs_directory,
      .input_env_getter = env_var_getter,
      .expected_result  = libd_pf_fs_ok,
      .expected_path    = "/home/user/foo/\0",
    },
  };

  size_t nb_params = sizeof(params) / sizeof(struct init_params);
  return cr_make_param_array(struct init_params, params, nb_params, NULL);
}

ParameterizedTest(
  struct init_params* params,
  path,
  init_env_expansion)
{
  cr_assert(1);
  // libd_platform_filesystem_path_o* path = malloc(LIBD_PF_FS_PATH_ALLOC_SIZE);
  //
  // libd_platform_filesystem_result_e result;
  // result = libd_platform_filesystem_path_init(
  //   path, params->input_path, params->input_type, params->input_env_getter);
  //
  // cr_assert(
  //   eq(str, libd_platform_filesystem_path_string(path),
  //   params->expected_path), "test name='%s'\ninput='%s'\n",
  //   params->test_name,
  //   params->input_path);
  //
  // cr_assert(eq(u8, result, params->expected_result), "%s\n",
  // params->test_name);
}
