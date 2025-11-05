#ifndef FILESYSTEM_FILEPATH_H
#define FILESYSTEM_FILEPATH_H

#include "../../include/libd/filesystem.h"
#include "./internal/allocator_wrap.h"

enum filepath_result {
  filepath_result_count,
};

// smaller footprint
#define separator_type 0
#define component_type 1
#define eof_type       2
typedef u8 token_type_e;

struct path_token_node {
  struct path_token_node* prev;
  struct path_token_node* next;
  token_type_e type;
  u8 val_len;
  char value[];
};

struct filepath_allocator {
  struct filepath_allocator_wrapper wrapper;
  alloc_f alloc;
  reset_f reset;
};

struct filepath_resolver {
  const char* src;
  enum libd_filesystem_path_type path_type;
  struct path_token_node* head;
  struct filepath_allocator allocator;
};

struct filepath {
  token_type_e types[8];
  char* values[8];
  size_t length;
  struct filepath_allocator allocator;
};

enum libd_result
libd_filepath_resolver_create(
  struct filepath_resolver** out_fpr,
  const char* src_path,
  enum libd_filesystem_path_type path_type);

void
libd_filepath_resolver_destroy(struct filepath_resolver* fpr);

enum libd_result
libd_filepath_resolver_tokenize(struct filepath_resolver* fpr);

enum libd_result
libd_filepath_resolver_expand(
  struct filepath_resolver* fpr,
  libd_filesystem_env_get_f env_getter);

enum libd_result
libd_filepath_resolver_normalize(struct filepath_resolver* fpr);

enum libd_result
libd_filepath_resolver_dump_to_filepath(
  struct filepath_resolver* fpr,
  struct filepath* dest);

enum libd_result
libd_filepath_allocator_create(
  struct filepath_allocator* out_pta,
  u8 alignment);

void
libd_filepath_allocator_destroy(struct filepath_allocator* pta);

#endif  // FILESYSTEM_FILEPATH_H
