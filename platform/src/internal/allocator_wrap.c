#include "./internal.h"

#include <libdane/memory.h>

enum libd_allocator_result
libd_allocator_wrapper_create(
  struct libd_allocator_wrapper* allocator,
  size_t size,
  uint8_t alignment)
{
  libd_memory_result_e r =
    libd_memory_linear_allocator_create(&allocator->a, size, alignment);
  if (r != libd_mem_ok) {
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
  libd_memory_result_e r =
    libd_memory_linear_allocator_alloc(allocator->a, &out_ptr, bytes);
  if (r != libd_mem_ok) {
    return NULL;
  }
  return out_ptr;
}

void
libd_allocator_wrapper_reset(struct libd_allocator_wrapper* allocator)
{
  libd_memory_linear_allocator_reset(allocator->a);
}

void
libd_allocator_wrapper_destroy(struct libd_allocator_wrapper* allocator)
{
  libd_memory_linear_allocator_destroy(allocator->a);
  allocator->a = NULL;
}
