#include "../../../include/libd/platform/filesystem.h"
#include "../../../include/libd/testing.h"

TEST(path_tokenizer_run)
{
  struct test_path_token {
    token_type_e type;
    char value[32];
  };
  struct test_case {
    char src[128];
    enum libd_platform_filesystem_path_type path_type;
    size_t expected_token_stream_length;
    struct test_path_token expected_token_stream[64];
  };
  struct test_case tcs[] = {
    {
      .src = "/\0",
      .path_type = libd_pf_fs_abs_directory,
      .expected_token_stream_length = 2,
      .expected_token_stream = {
        {
          .type = separator_type,
          .value = PATH_SEPARATOR_VALUE,
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
          .value = PATH_SEPARATOR_VALUE,
        },
        {
          .type  = component_type,
          .value = "home\0",
        },
        {
          .type  = separator_type,
          .value = PATH_SEPARATOR_VALUE,
        },
        {
          .type  = component_type,
          .value = "user\0",
        },
        {
          .type  = separator_type,
          .value = PATH_SEPARATOR_VALUE,
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

  size_t num_tests = ARR_LEN(tcs);
  for (size_t i = 0; i < num_tests; i += 1) {
    struct libd_path_tokenizer* pt;
    ASSERT_OK(libd_path_tokenizer_create(&pt, tcs[i].src, tcs[i].path_type));
    ASSERT_OK(libd_path_tokenizer_run(pt));
    ASSERT_EQ_U(pt->token_count, tcs[i].expected_token_stream_length);

    for (size_t j = 0; j < pt->token_count; j += 1) {
      ASSERT_EQ_U(
        pt->token_stream[j]->type, tcs[i].expected_token_stream[j].type);
      ASSERT_EQ_STR(
        pt->token_stream[j]->value, tcs[i].expected_token_stream[j].value);
    }

    libd_path_tokenizer_destroy(pt);
  }
}
