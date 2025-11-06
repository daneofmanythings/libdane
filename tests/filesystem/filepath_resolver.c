#include "../../include/libd/filesystem.h"
#include "../../include/libd/platform/filesystem.h"
#include "../../include/libd/testing.h"
#include "../../src/filesystem/filepath.h"

#include <stddef.h>
#include <string.h>

static struct filepath_resolver*
helper_filepath_resolver_create(
  const char* src,
  enum libd_filesystem_path_type type)
{
  struct filepath_resolver* fpr;

  ASSERT_OK(libd_filepath_resolver_create(&fpr, src, type));

  return fpr;
}

TEST(filepath_resolver_create_destroy)
{
  struct test_case {
    char* name;
    char* src;
    enum libd_filesystem_path_type type;
    enum libd_result expected;
  } tcs[] = {
    {
      .name     = "fine\0",
      .src      = "a\0",
      .type     = libd_rel_file,
      .expected = libd_ok,
    },
    {
      .name     = "null path\0",
      .src      = NULL,
      .type     = libd_filesystem_path_type_count,
      .expected = libd_invalid_parameter,
    },
    {
      .name     = "empty path\0",
      .src      = "\0",
      .type     = libd_filesystem_path_type_count,
      .expected = libd_invalid_parameter,
    },
  };

  ASSERT_EQ_U(
    libd_filepath_resolver_create(NULL, "a\0", libd_rel_file),
    libd_invalid_parameter);

  struct filepath_resolver* fpr = NULL;
  enum libd_result r;
  for (u8 i = 0; i < ARR_LEN(tcs); i++) {
    r = libd_filepath_resolver_create(&fpr, tcs[i].src, tcs[i].type);
    ASSERT_EQ_U(r, tcs[i].expected);
    if (fpr != NULL) {
      libd_filepath_resolver_destroy(fpr);
      fpr = NULL;
    }
  }
}

TEST(filepath_resolver_tokenize)
{
  struct token {
    u8 type;
    char* val;
  };

  struct test_case {
    char* name;
    char* input_src;
    u8 expected_token_count;
    struct token expected_tokens[16];

  } tcs[] = {
    {
      .name      = "basic absolute\0",
      .input_src = "/home/user\0",
      .expected_token_count = 5,
      .expected_tokens= {
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE},
        {.type = component_type, .val="home\0"},
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE},
        {.type = component_type, .val="user\0"},
        {.type=eof_type, .val="\0"},
      },
    },
    {
      .name      = "nonsense path\0",
      .input_src = "////-fda93.,/=d-s\0",
      .expected_token_count = 7,
      .expected_tokens= {
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE},
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE},
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE},
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE},
        {.type = component_type, .val="-fda93.,\0"},
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE},
        {.type = component_type, .val="=d-s\0"},
        {.type=eof_type, .val="\0"},
      },
    },
  };

  struct filepath_resolver* fpr;
  for (size_t i = 0; i < ARR_LEN(tcs); i += 1) {

    fpr = helper_filepath_resolver_create(tcs[i].input_src, libd_rel_file);

    ASSERT_OK(libd_filepath_resolver_tokenize(fpr));
    struct path_token_node* head = fpr->head;

    for (size_t j = 0; j < tcs->expected_token_count; j += 1) {
      ASSERT_EQ_U(head->type, tcs[i].expected_tokens[j].type);
      ASSERT_EQ_STR(head->value, tcs[i].expected_tokens[j].val);
      head = head->next;
    }

    libd_filepath_resolver_destroy(fpr);
  }
}

enum libd_result
test_env_getter(
  char* out_val,
  const char* key);

TEST(filepath_resolver_expand)
{
  struct token {
    u8 type;
    char* val;
  };

  struct test_case {
    char* name;
    char* input_src;
    u8 expected_token_count;
    struct token expected_tokens[16];
  } tcs[] = {
    {
      .name="one expansion\0",
      .input_src = "/$one\0", // FIX: this test is posix specific :(
      .expected_token_count = 3,
      .expected_tokens = {
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE },
        {.type=component_type, .val="zero\0" },
        {.type=eof_type, .val = "\0" },
      },
    },
    {
      .name="two expansions\0",
      .input_src = "/$two\0", // FIX: this test is posix specific :(
      .expected_token_count = 5,
      .expected_tokens = {
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE },
        {.type=component_type, .val="zero\0" },
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE },
        {.type=component_type, .val="zero\0" },
        {.type=eof_type, .val = "\0" },
      },
    },
    {
      .name="three expansions\0",
      .input_src = "$three/\0", // FIX: this test is posix specific :(
      .expected_token_count = 7,
      .expected_tokens = {
        {.type=component_type, .val="zero\0" },
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE },
        {.type=component_type, .val="zero\0" },
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE },
        {.type=component_type, .val="zero\0" },
        {.type=separator_type, .val=PATH_SEPARATOR_VALUE },
        {.type=eof_type, .val = "\0" },
      },
    },
  };

  struct filepath_resolver* fpr;
  for (size_t i = 0; i < ARR_LEN(tcs); i += 1) {
    fpr = helper_filepath_resolver_create(tcs[i].input_src, libd_rel_file);

    ASSERT_OK(libd_filepath_resolver_tokenize(fpr));
    ASSERT_OK(libd_filepath_resolver_expand(fpr, test_env_getter));

    struct path_token_node* head = fpr->head;

    for (size_t j = 0; j < tcs->expected_token_count; j += 1) {
      ASSERT_NOT_NULL(head);
      ASSERT_EQ_U(head->type, tcs[i].expected_tokens[j].type);
      ASSERT_EQ_STR(head->value, tcs[i].expected_tokens[j].val);
      head = head->next;
    }

    libd_filepath_resolver_destroy(fpr);
  }
}

enum libd_result
test_env_getter(
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
