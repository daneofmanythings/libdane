#include "../include/libdane/memory.h"

#include <stdint.h>
#include <stdlib.h>

struct libd_memory_linear_allocator_s {
  size_t  capacity;
  size_t  bytes_index;
  uint8_t alignment;
  uint8_t dat[];
};

typedef libd_memory_linear_allocator_o           linear_allocator;
typedef libd_memory_linear_allocator_savepoint_s savepoint;

struct libd_memory_linear_allocator_savepoint_s {
  size_t bytes_index;
};
typedef struct libd_memory_linear_allocator_savepoint_s savepoint_s;

libd_memory_result_e
libd_memory_linear_allocator_create(
  linear_allocator** out_allocator,
  size_t             capacity_bytes,
  uint8_t            alignment)
{
  if (out_allocator == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }
  if (capacity_bytes == 0) {
    return LIBD_MEM_INVALID_ZERO_PARAMETER;
  }

  libd_memory_result_e result = libd_memory_is_valid_alignment(alignment);
  if (result != LIBD_MEM_OK) {
    return result;
  }

  size_t allocation_size = sizeof(linear_allocator) + capacity_bytes;
  allocation_size        = libd_memory_align_value(allocation_size, alignment);

  linear_allocator* p_allocator = malloc(allocation_size);
  if (p_allocator == NULL) {
    return LIBD_MEM_NO_MEMORY;
  }

  p_allocator->capacity    = capacity_bytes;
  p_allocator->bytes_index = 0;
  p_allocator->alignment   = alignment;

  *out_allocator = p_allocator;

  return LIBD_MEM_OK;
}

libd_memory_result_e
libd_memory_linear_allocator_destroy(linear_allocator* p_allocator)
{
  if (p_allocator == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }

  free(p_allocator);

  return LIBD_MEM_OK;
}

libd_memory_result_e
libd_memory_linear_allocator_alloc(
  linear_allocator* p_allocator,
  void**            out_pointer,
  size_t            size)
{
  if (p_allocator == NULL || out_pointer == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }
  if (p_allocator->capacity < p_allocator->bytes_index + size) {
    return LIBD_MEM_NO_MEMORY;
  }

  // This state should not be possible.
  size_t aligned_index =
    libd_memory_align_value(p_allocator->bytes_index, p_allocator->alignment);
  if (aligned_index != p_allocator->bytes_index) {
    return LIBD_MEM_INVALID_ALIGNMENT;
  }
  // -----

  *out_pointer = &p_allocator->dat[p_allocator->bytes_index];
  p_allocator->bytes_index +=
    libd_memory_align_value(size, p_allocator->alignment);

  return LIBD_MEM_OK;
}

libd_memory_result_e
libd_memory_linear_allocator_set_savepoint(
  const linear_allocator* p_allocator,
  savepoint*              out_savepoint)
{
  if (p_allocator == NULL || out_savepoint == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }

  *out_savepoint = (savepoint_s){ .bytes_index = p_allocator->bytes_index };

  return LIBD_MEM_OK;
}

libd_memory_result_e
libd_memory_linear_allocator_restore_savepoint(
  linear_allocator* p_allocator,
  const savepoint*  p_savepoint)
{
  if (p_allocator == NULL || p_savepoint == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }

  p_allocator->bytes_index = p_savepoint->bytes_index;

  return LIBD_MEM_OK;
}

libd_memory_result_e
libd_memory_linear_allocator_reset(linear_allocator* p_allocator)
{
  if (p_allocator == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }

  p_allocator->bytes_index = 0;

  return LIBD_MEM_OK;
}
