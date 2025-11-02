#include "../../../memory/include/libdane/memory.h"
#include "./internal.h"

enum libd_allocator_result
libd_allocator_wrapper_create(
  struct libd_allocator_wrapper* allocator,
  size_t size,
  uint8_t alignment)
{
  enum libd_result r =
    libd_linear_allocator_create(&allocator->a, size, alignment);
  if (r != libd_ok) {
    return enomem;
  }

  return ok;
}

void*
libd_allocator_wrapper_alloc(
  struct libd_allocator_wrapper* allocator,
  size_t bytes)
{
  void* out_ptr;
  enum libd_result r =
    libd_linear_allocator_alloc(allocator->a, &out_ptr, bytes);
  if (r != libd_ok) {
    return NULL;
  }
  return out_ptr;
}

void
libd_allocator_wrapper_reset(struct libd_allocator_wrapper* allocator)
{
  libd_linear_allocator_reset(allocator->a);
}

void
libd_allocator_wrapper_destroy(struct libd_allocator_wrapper* allocator)
{
  libd_linear_allocator_destroy(allocator->a);
  allocator->a = NULL;
}
