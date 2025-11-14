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

struct filepath_allocator {
  struct filepath_allocator_wrapper wrapper;
  alloc_f alloc;
  reset_f reset;
};

enum libd_result
libd_filepath_allocator_create(
  struct filepath_allocator* out_fpa,
  u8 alignment);

void
libd_filepath_allocator_destroy(struct filepath_allocator* fpa);

#endif  // FILESYSTEM_FILEPATH_H
