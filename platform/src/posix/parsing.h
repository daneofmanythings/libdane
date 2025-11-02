#include "../../include/libdane/platform/filesystem.h"
#include "../internal/internal.h"

#include <stdint.h>

struct libd_token_allocator {
  struct libd_allocator_wrapper allocator;
  alloc_f alloc;
  reset_f reset;
};

#define separator_type 0
#define component_type 1
#define eof_type       2
typedef uint8_t token_type_e;
struct libd_path_token {
  token_type_e type;
  char value[];
};

struct libd_path_tokenizer {
  const char* src;
  enum libd_platform_filesystem_path_type path_type;
  struct libd_token_allocator token_allocator;
  struct libd_path_token* token_stream[4096];
  size_t token_count;
  size_t scan_pos;
};

enum libd_allocator_result
libd_path_tokenizer_create(
  struct libd_path_tokenizer** out_pt,
  const char* src_path,
  enum libd_platform_filesystem_path_type path_type);

void
libd_path_tokenizer_destroy(struct libd_path_tokenizer* pt);

enum libd_allocator_result
libd_path_tokenizer_run(struct libd_path_tokenizer* pt);

enum libd_allocator_result
libd_path_tokenizer_evaluate(
  struct libd_path_tokenizer* pt,
  char* dest);
