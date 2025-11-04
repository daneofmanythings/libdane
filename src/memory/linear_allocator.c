#include "../../include/libd/memory.h"
#include "./internal/helpers.h"

#include <stdint.h>
#include <stdlib.h>

struct linear_allocator {
  size_t capacity;
  size_t head_index_bytes;
  uint8_t alignment;
  uint8_t* data;
};

struct linear_allocator_savepoint {
  size_t index_bytes;
};

enum libd_result
libd_linear_allocator_create(
  struct linear_allocator** out_allocator,
  size_t capacity_bytes,
  uint8_t alignment)
{
  if (out_allocator == NULL || capacity_bytes == 0) {
    return libd_invalid_parameter;
  }

  enum libd_result result = libd_memory_is_valid_alignment(alignment);
  if (result != libd_ok) {
    return result;
  }

  size_t allocation_size =
    sizeof(struct linear_allocator) + capacity_bytes + alignment - 1;
  allocation_size = libd_memory_align_value(allocation_size, alignment);

  struct linear_allocator* p_allocator = malloc(allocation_size);
  if (p_allocator == NULL) {
    return libd_no_memory;
  }

  p_allocator->capacity         = capacity_bytes;
  p_allocator->head_index_bytes = 0;
  p_allocator->alignment        = alignment;

  // setting the start of the data region to the given alignment.
  uintptr_t aligned_data_start = libd_memory_align_value(
    (uintptr_t)p_allocator + sizeof(struct linear_allocator), alignment);
  p_allocator->data = (uint8_t*)aligned_data_start;

  *out_allocator = p_allocator;

  return libd_ok;
}

enum libd_result
libd_linear_allocator_destroy(struct linear_allocator* p_allocator)
{
  if (p_allocator == NULL) {
    return libd_invalid_parameter;
  }

  free(p_allocator);

  return libd_ok;
}

enum libd_result
libd_linear_allocator_alloc(
  struct linear_allocator* p_allocator,
  void** out_pointer,
  size_t size)
{
  if (p_allocator == NULL || out_pointer == NULL) {
    return libd_invalid_parameter;
  }

  if (p_allocator->capacity < p_allocator->head_index_bytes + size) {
    return libd_no_memory;
  }

  // This state should not be possible.
  size_t aligned_index = libd_memory_align_value(
    p_allocator->head_index_bytes, p_allocator->alignment);
  if (aligned_index != p_allocator->head_index_bytes) {
    return libd_invalid_alignment;
  }
  // -----

  *out_pointer = &p_allocator->data[p_allocator->head_index_bytes];
  p_allocator->head_index_bytes +=
    libd_memory_align_value(size, p_allocator->alignment);

  return libd_ok;
}

enum libd_result
libd_linear_allocator_set_savepoint(
  const struct linear_allocator* p_allocator,
  struct linear_allocator_savepoint* out_savepoint)
{
  if (p_allocator == NULL || out_savepoint == NULL) {
    return libd_invalid_parameter;
  }

  *out_savepoint = (struct linear_allocator_savepoint){
    .index_bytes = p_allocator->head_index_bytes,
  };

  return libd_ok;
}

enum libd_result
libd_linear_allocator_restore_savepoint(
  struct linear_allocator* p_allocator,
  const struct linear_allocator_savepoint* p_savepoint)
{
  if (p_allocator == NULL || p_savepoint == NULL) {
    return libd_invalid_parameter;
  }

  p_allocator->head_index_bytes = p_savepoint->index_bytes;

  return libd_ok;
}

enum libd_result
libd_linear_allocator_reset(struct linear_allocator* p_allocator)
{
  if (p_allocator == NULL) {
    return libd_invalid_parameter;
  }

  p_allocator->head_index_bytes = 0;

  return libd_ok;
}

enum libd_result
libd_linear_allocator_bytes_free(
  libd_linear_allocator_h* p_allocator,
  size_t* out_size_bytes)
{
  if (p_allocator == NULL) {
    return libd_invalid_parameter;
  }

  *out_size_bytes = p_allocator->capacity - p_allocator->head_index_bytes;

  return libd_ok;
}
