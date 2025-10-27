#include "../include/libdane/memory.h"

#include <stdint.h>
#include <stdlib.h>

typedef libd_memory_linear_allocator_ot linear_allocator;
typedef libd_memory_result_e result_e;
typedef libd_memory_linear_allocator_savepoint_ot savepoint;

struct libd_memory_linear_allocator_s {
  size_t capacity;
  size_t head_index_bytes;
  uint8_t alignment;
  uint8_t* data;
};

struct libd_memory_linear_allocator_savepoint_s {
  size_t index_bytes;
};

result_e
libd_memory_linear_allocator_create(
  linear_allocator** out_allocator,
  size_t capacity_bytes,
  uint8_t alignment)
{
  if (out_allocator == NULL) {
    return libd_mem_invalid_null_parameter;
  }
  if (capacity_bytes == 0) {
    return libd_mem_invalid_zero_parameter;
  }

  result_e result = libd_memory_is_valid_alignment(alignment);
  if (result != libd_mem_ok) {
    return result;
  }

  size_t allocation_size =
    sizeof(linear_allocator) + capacity_bytes + alignment - 1;
  allocation_size = libd_memory_align_value(allocation_size, alignment);

  linear_allocator* p_allocator = malloc(allocation_size);
  if (p_allocator == NULL) {
    return libd_mem_no_memory;
  }

  p_allocator->capacity         = capacity_bytes;
  p_allocator->head_index_bytes = 0;
  p_allocator->alignment        = alignment;

  // setting the start of the data region to the given alignment.
  uintptr_t aligned_data_start = libd_memory_align_value(
    (uintptr_t)p_allocator + sizeof(linear_allocator), alignment);
  p_allocator->data = (uint8_t*)aligned_data_start;

  *out_allocator = p_allocator;

  return libd_mem_ok;
}

result_e
libd_memory_linear_allocator_destroy(linear_allocator* p_allocator)
{
  if (p_allocator == NULL) {
    return libd_mem_invalid_null_parameter;
  }

  free(p_allocator);

  return libd_mem_ok;
}

result_e
libd_memory_linear_allocator_alloc(
  linear_allocator* p_allocator,
  void** out_pointer,
  size_t size)
{
  if (p_allocator == NULL || out_pointer == NULL) {
    return libd_mem_invalid_null_parameter;
  }
  if (p_allocator->capacity < p_allocator->head_index_bytes + size) {
    return libd_mem_no_memory;
  }

  // This state should not be possible.
  size_t aligned_index = libd_memory_align_value(
    p_allocator->head_index_bytes, p_allocator->alignment);
  if (aligned_index != p_allocator->head_index_bytes) {
    return libd_mem_invalid_alignment;
  }
  // -----

  *out_pointer = &p_allocator->data[p_allocator->head_index_bytes];
  p_allocator->head_index_bytes +=
    libd_memory_align_value(size, p_allocator->alignment);

  return libd_mem_ok;
}

result_e
libd_memory_linear_allocator_set_savepoint(
  const linear_allocator* p_allocator,
  savepoint* out_savepoint)
{
  if (p_allocator == NULL || out_savepoint == NULL) {
    return libd_mem_invalid_null_parameter;
  }

  *out_savepoint = (savepoint){ .index_bytes = p_allocator->head_index_bytes };

  return libd_mem_ok;
}

result_e
libd_memory_linear_allocator_restore_savepoint(
  linear_allocator* p_allocator,
  const savepoint* p_savepoint)
{
  if (p_allocator == NULL || p_savepoint == NULL) {
    return libd_mem_invalid_null_parameter;
  }

  p_allocator->head_index_bytes = p_savepoint->index_bytes;

  return libd_mem_ok;
}

result_e
libd_memory_linear_allocator_reset(linear_allocator* p_allocator)
{
  if (p_allocator == NULL) {
    return libd_mem_invalid_null_parameter;
  }

  p_allocator->head_index_bytes = 0;

  return libd_mem_ok;
}
