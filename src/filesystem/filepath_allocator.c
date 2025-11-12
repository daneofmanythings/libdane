#include "filepath.h"

enum libd_result
libd_filepath_allocator_create(
  struct filepath_allocator* out_ta,
  u8 alignment)
{
  enum libd_result r =
    libd_allocator_wrapper_create(&out_ta->wrapper, 8 * KiB, KiB, alignment);
  if (r != libd_ok) {
    return libd_no_memory;
  }

  out_ta->alloc = libd_allocator_wrapper_alloc;
  out_ta->reset = libd_allocator_wrapper_reset;

  return libd_ok;
}

void
libd_filepath_allocator_destroy(struct filepath_allocator* ta)
{
  libd_allocator_wrapper_destroy(&ta->wrapper);
}
