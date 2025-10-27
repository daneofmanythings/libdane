#include "../../../include/libdane/platform/filesystem.h"

#include <stdlib.h>

typedef libd_platform_filesystem_result_e result_e;
typedef libd_platform_filesystem_path_type_e path_type_e;

#define PATH_BUFFER_MAX (LIBD_PF_FS_PATH_MAX + 1)
#define WORK_BUFFER_MAX (2 * LIBD_PF_FS_PATH_MAX + 1)

enum result {
  ok,
  bad,
};

#define separator_type 0
#define component_type 1
#define eof_type       2
struct path_token {
  uint8_t type;
  char* value;
};

struct path_tokenizer {
  const char* source;
  path_type_e path_type;
  struct token_allocator* token_allocator;
  struct path_token token_stream[4096];
  size_t token_pointers[4096];
};

enum result
path_tokenizer_create(
  struct path_tokenizer** out_pt,
  const char* src_path,
  path_type_e path_type)
{
  enum result r = ok;

  struct path_tokenizer* pt = malloc(sizeof(*pt));
  if (pt == NULL) {
    return bad;
  }

  pt->source    = src_path;
  pt->path_type = path_type;

  r = token_allocator_create(pt->token_allocator);
  if (r != 0) {
    free(pt);
    return r;
  }

  return r;
}

enum result
path_tokenizer_run(struct path_tokenizer* pt)
{
  //
}

result_e
normalize_path_string(
  char dest_path[WORK_BUFFER_MAX],
  const char src_path[WORK_BUFFER_MAX],
  path_type_e path_type)
{
  enum result r = ok;
  // setup tokenizer
  struct path_tokenizer* t;
  r = path_tokenizer_create(&t, src_path, path_type);

  // run tokenizer
  r = path_tokenizer_run(t);
  if (r != ok) {
    goto cleanup;
  }

  // run evaluator
  r = path_tokenizer_evaluate(t, dest_path);
  if (r != ok) {
    goto cleanup;
  }

cleanup:
  path_tokenizer_destroy(t);
  return r;
}
