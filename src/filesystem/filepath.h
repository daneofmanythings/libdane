#ifndef FILESYSTEM_FILEPATH_H
#define FILESYSTEM_FILEPATH_H

#include "../../include/libd/common.h"
#include "../../include/libd/filesystem.h"
#include "./internal/allocator_wrap.h"

typedef u8 token_type_e;
#define eof_type                 (1 << 0)
#define separator_type           (1 << 1)
#define parent_ref_type          (1 << 2)
#define self_ref_type            (1 << 3)
#define segment_type             (1 << 4)
#define component_mask           (parent_ref_type | self_ref_type | segment_type)
#define IS_EOF_TYPE(type)        CHECK_AGAINST_MASK(type, eof_type)
#define IS_SEPARATOR_TYPE(type)  CHECK_AGAINST_MASK(type, separator_type)
#define IS_PARENT_REF_TYPE(type) CHECK_AGAINST_MASK(type, parent_ref_type)
#define IS_SELF_REF_TYPE(type)   CHECK_AGAINST_MASK(type, self_ref_type)
#define IS_SEGMENT_TYPE(type)    CHECK_AGAINST_MASK(type, segment_type)
#define IS_COMPONENT_TYPE(type)  CHECK_AGAINST_MASK(type, component_mask)

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

#define FILEPATH_COMPONENT_MAX 16
struct filepath {
  token_type_e types[FILEPATH_COMPONENT_MAX];
  char* values[FILEPATH_COMPONENT_MAX];
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
  struct filepath_allocator* out_fpa,
  u8 alignment);

void
libd_filepath_allocator_destroy(struct filepath_allocator* fpa);

#endif  // FILESYSTEM_FILEPATH_H
