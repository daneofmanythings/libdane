#include "../../src/posix/filesystem/parsing.h"

#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <criterion/new/assert.h>
#include <criterion/parameterized.h>

#ifdef _WIN32
  #define SEPARATOR_VALUE "\\\\\0"
#else
  #define SEPARATOR_VALUE "/\0"
#endif

#define EOF_VALUE "\0"

struct test_path_token {
  token_type_e type;
  char value[32];
};
struct pt_run_param {
  char src[128];
  libd_platform_filesystem_path_type_e path_type;
  size_t expected_token_stream_length;
  struct test_path_token expected_token_stream[64];
};
ParameterizedTestParameters(
  path_tokenizer,
  run)
{
  static struct pt_run_param params[] = {
    {
      .src = "/\0",
      .path_type = libd_pf_fs_abs_directory,
      .expected_token_stream_length = 2,
      .expected_token_stream = {
        {
          .type = separator_type,
          .value = SEPARATOR_VALUE,
        },
        {
          .type = eof_type,
          .value = EOF_VALUE,
        },
      },
    },
    {
      .src = "/home/user/test\0",
      .path_type = libd_pf_fs_rel_file,
      .expected_token_stream_length = 7,
      .expected_token_stream = {
        {
          .type  = separator_type,
          .value = SEPARATOR_VALUE,
        },
        {
          .type  = component_type,
          .value = "home\0",
        },
        {
          .type  = separator_type,
          .value = SEPARATOR_VALUE,
        },
        {
          .type  = component_type,
          .value = "user\0",
        },
        {
          .type  = separator_type,
          .value = SEPARATOR_VALUE,
        },
        {
          .type  = component_type,
          .value = "test\0",
        },
        {
          .type  = eof_type,
          .value = EOF_VALUE,
        },
      },
    },
  };

  size_t nb_params = sizeof(params) / sizeof(struct pt_run_param);
  return cr_make_param_array(struct pt_run_param, params, nb_params, NULL);
}

ParameterizedTest(
  struct pt_run_param* param,
  path_tokenizer,
  run)
{
  struct libd_path_tokenizer* pt;
  cr_assert(
    eq(u8, libd_path_tokenizer_create(&pt, param->src, param->path_type), ok));
  cr_assert(eq(u8, libd_path_tokenizer_run(pt), ok));
  cr_assert(eq(u64, pt->token_count, param->expected_token_stream_length));

  for (size_t i = 0; i < pt->token_count; i += 1) {
    cr_assert(
      eq(u8, pt->token_stream[i]->type, param->expected_token_stream[i].type),
      "tok=%u, type=%u, expected=%u, value=%s\n",
      i,
      pt->token_stream[i]->type,
      param->expected_token_stream[i].type,
      pt->token_stream[i]->value);
    cr_assert(
      eq(
        str, pt->token_stream[i]->value, param->expected_token_stream[i].value),
      "result=%s, expected=%s\n",
      pt->token_stream[i]->value,
      param->expected_token_stream[i].value);
  }

  libd_path_tokenizer_destroy(pt);
}
