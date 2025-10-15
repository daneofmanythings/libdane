#include "../include/libdane/internal/align_compat.h"
#include "../include/libdane/memory.h"

#include <stdint.h>
#include <stdlib.h>


struct libd_memory_linear_allocator_s {
  size_t capacity;
  size_t bytes_index;
  uint8_t alignment;
  uint8_t dat[];
};
typedef libd_memory_linear_allocator_s linear_allocator_s;

struct libd_memory_linear_allocator_savepoint_s {
  size_t bytes_index;
};
typedef struct libd_memory_linear_allocator_savepoint_s savepoint_s;

static libd_memory_result_e
_validate_alignment(uint8_t alignment);
libd_memory_result_e
libd_memory_linear_allocator_create(linear_allocator_s** out_allocator,
                                    size_t capacity_bytes,
                                    uint8_t alignment)
{
  if (out_allocator == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }
  if (capacity_bytes == 0) {
    return ERR_INVALID_ZERO_PARAMETER;
  }

  libd_memory_result_e result = _validate_alignment(alignment);
  if (result != RESULT_OK) {
    return result;
  }

  size_t allocation_size = sizeof(linear_allocator_s) + capacity_bytes;
  allocation_size = libd_memory_align_value(allocation_size, alignment);

  linear_allocator_s* p_allocator = malloc(allocation_size);
  if (p_allocator == NULL) {
    return ERR_NO_MEMORY;
  }

  p_allocator->capacity = capacity_bytes;
  p_allocator->bytes_index = 0;
  p_allocator->alignment = alignment;

  *out_allocator = p_allocator;

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_linear_allocator_destroy(linear_allocator_s* p_allocator)
{
  if (p_allocator == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }

  free(p_allocator);

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_linear_allocator_alloc(linear_allocator_s* p_allocator,
                                   void** out_pointer,
                                   size_t size)
{
  if (p_allocator == NULL || out_pointer == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }
  if (p_allocator->capacity < p_allocator->bytes_index + size) {
    return ERR_NO_MEMORY;
  }
  size_t aligned_index =
    libd_memory_align_value(p_allocator->bytes_index, p_allocator->alignment);
  if (aligned_index != p_allocator->bytes_index) {
    return ERR_INVALID_ALIGNMENT;
  }

  *out_pointer = &p_allocator->dat[p_allocator->bytes_index];
  p_allocator->bytes_index +=
    libd_memory_align_value(size, p_allocator->alignment);

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_linear_allocator_set_savepoint(
  const libd_memory_linear_allocator_s* p_allocator,
  libd_memory_linear_allocator_savepoint_s* out_savepoint)
{
  if (p_allocator == NULL || out_savepoint == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }

  *out_savepoint = (savepoint_s){.bytes_index = p_allocator->bytes_index};

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_linear_allocator_restore_savepoint(
  libd_memory_linear_allocator_s* p_allocator,
  const libd_memory_linear_allocator_savepoint_s* p_savepoint)
{
  if (p_allocator == NULL || p_savepoint == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }

  p_allocator->bytes_index = p_savepoint->bytes_index;

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_linear_allocator_reset(linear_allocator_s* p_allocator)
{
  if (p_allocator == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }

  p_allocator->bytes_index = 0;

  return RESULT_OK;
}

static libd_memory_result_e
_validate_alignment(uint8_t alignment)
{
  if (alignment == 0) {
    return ERR_INVALID_ZERO_PARAMETER;
  }

  if (!libd_memory_is_power_of_two(alignment) || alignment > LIBD_MAX_ALIGN) {
    return ERR_INVALID_ALIGNMENT;
  }

  return RESULT_OK;
}
