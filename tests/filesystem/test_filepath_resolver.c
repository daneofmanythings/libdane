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

void
test_filepath_resolver_dump_path_string(
  struct filepath_resolver* fpr,
  char* dest);

TEST(filepath_resolver_expand)
{
  struct test_case {
    char* name;
    char* input_src;
    const char* expected_value;
  } tcs[] = {
    {
      .name           = "one expansion\0",
      .input_src      = "/$one\0",
      .expected_value = "/zero\0",
    },
    {
      .name           = "two expansions\0",
      .input_src      = "/$two\0",
      .expected_value = "/zero/zero\0",
    },
    {
      .name           = "three expansions\0",
      .input_src      = "$three/\0",
      .expected_value = "zero/zero/zero/\0",
    },
    {
      .name           = "shell syntax\0",
      .input_src      = "${three}/\0",
      .expected_value = "zero/zero/zero/\0",
    },
    {
      .name           = "shell syntax with default\0",
      .input_src      = "${four:-zero}/\0",
      .expected_value = "zero/\0",
    },
    {
      .name           = "shell syntax with interpolated default\0",
      .input_src      = "${four:-$three}/\0",
      .expected_value = "zero/zero/zero/\0",
    },
  };

  struct filepath_resolver* fpr;
  char test_dest[128] = { 0 };
  for (size_t i = 0; i < ARR_LEN(tcs); i += 1) {
    fpr = helper_filepath_resolver_create(tcs[i].input_src, libd_rel_file);

    ASSERT_OK(libd_filepath_resolver_tokenize(fpr));
    ASSERT_OK(libd_filepath_resolver_expand(fpr, test_env_getter));

    test_filepath_resolver_dump_path_string(fpr, test_dest);
    ASSERT_EQ_STR(test_dest, tcs[i].expected_value);

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

void
test_filepath_resolver_dump_path_string(
  struct filepath_resolver* fpr,
  char* dest)
{
  size_t offset                = 0;
  struct path_token_node* curr = fpr->head;
  while (curr->type != eof_type) {
    strcpy(dest + offset, curr->value);
    offset += curr->val_len;
    curr = curr->next;
  }

  *(dest + offset) = '\0';
}

TEST(filepath_resolver_normalize)
{
  struct test_case {
    char* name;
    char* input_src;
    enum libd_filesystem_path_type input_type;
    enum libd_result expected_code;
    const char* expected_value;
  } tcs[] = {
    // {
    //   .name           = "remove extra separators 1\0",
    //   .input_src      = "//zero\0",
    //   .input_type     = libd_abs_file,
    //   .expected_code  = libd_ok,
    //   .expected_value = "/zero\0",
    // },
    // {
    //   .name           = "remove extra separators 2\0",
    //   .input_src      = "zero//\0",
    //   .input_type     = libd_rel_directory,
    //   .expected_code  = libd_ok,
    //   .expected_value = "zero/\0",
    // },
    // {
    //   .name           = "remove extra separators 3\0",
    //   .input_src      = "zero//zero\0",
    //   .input_type     = libd_rel_file,
    //   .expected_code  = libd_ok,
    //   .expected_value = "zero/zero\0",
    // },
    // {
    //   .name           = "remove extra separators 4\0",
    //   .input_src      = "/////zero////zero///zero\0",
    //   .input_type     = libd_abs_file,
    //   .expected_code  = libd_ok,
    //   .expected_value = "/zero/zero/zero\0",
    // },
    // {
    //   .name           = "remove self-refs 1\0",
    //   .input_src      = "./zero\0",
    //   .input_type     = libd_abs_file,
    //   .expected_code  = libd_ok,
    //   .expected_value = "/zero\0",
    // },
    // {
    //   .name           = "remove self-refs 2\0",
    //   .input_src      = "zero/./zero\0",
    //   .input_type     = libd_rel_file,
    //   .expected_code  = libd_ok,
    //   .expected_value = "zero/zero\0",
    // },
    // {
    //   .name           = "remove self-refs 3\0",
    //   .input_src      = "zero/./././zero\0",
    //   .input_type     = libd_rel_file,
    //   .expected_code  = libd_ok,
    //   .expected_value = "zero/zero\0",
    // },
    // {
    //   .name           = "remove self-refs 4\0",
    //   .input_src      = "zero/zero/./\0",
    //   .input_type     = libd_rel_directory,
    //   .expected_code  = libd_ok,
    //   .expected_value = "zero/zero/\0",
    // },
    // {
    //   .name           = "abs parent ref 1\0",
    //   .input_src      = "../zero\0",
    //   .input_type     = libd_abs_file,
    //   .expected_code  = libd_invalid_path,
    //   .expected_value = "\0",
    // },
    // {
    //   .name           = "abs parent ref 2\0",
    //   .input_src      = "/zero/../\0",
    //   .input_type     = libd_abs_file,
    //   .expected_code  = libd_invalid_path,
    //   .expected_value = "/\0",
    // },
    // {
    //   .name           = "abs parent ref 3\0",
    //   .input_src      = "/zero/zero/../../\0",
    //   .input_type     = libd_abs_file,
    //   .expected_code  = libd_invalid_path,
    //   .expected_value = "/\0",
    // },
    // {
    //   .name           = "abs parent ref 4\0",
    //   .input_src      = "/zero/../zero/../\0",
    //   .input_type     = libd_abs_file,
    //   .expected_code  = libd_invalid_path,
    //   .expected_value = "/\0",
    // },
    // {
    //   .name           = "rel parent ref 1\0",
    //   .input_src      = "../zero\0",
    //   .input_type     = libd_rel_file,
    //   .expected_code  = libd_ok,
    //   .expected_value = "../zero\0",
    // },
    // {
    //   .name           = "rel parent ref 2\0",
    //   .input_src      = "../zero/../\0",
    //   .input_type     = libd_rel_file,
    //   .expected_code  = libd_ok,
    //   .expected_value = "../\0",
    // },
    {
      .name           = "rel parent ref 3\0",
      .input_src      = "zero/../zero/../../\0",
      .input_type     = libd_rel_file,
      .expected_code  = libd_ok,
      .expected_value = "../\0",
    },
  };

  struct filepath_resolver* fpr;
  char test_dest[128] = { 0 };
  for (size_t i = 0; i < ARR_LEN(tcs); i += 1) {
    fpr = helper_filepath_resolver_create(tcs[i].input_src, tcs[i].input_type);

    ASSERT_OK(libd_filepath_resolver_tokenize(fpr));
    ASSERT_OK(libd_filepath_resolver_expand(fpr, test_env_getter));
    ASSERT_EQ_U(
      libd_filepath_resolver_normalize(fpr),
      tcs[i].expected_code,
      "at test name='%s'\n",
      tcs[i].name);

    if (tcs[i].expected_code == libd_ok) {
      test_filepath_resolver_dump_path_string(fpr, test_dest);
      ASSERT_EQ_STR(
        test_dest, tcs[i].expected_value, "at test name='%s'", tcs[i].name);
    }

    libd_filepath_resolver_destroy(fpr);
  }
}
