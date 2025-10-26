#include "../include/libdane/memory.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// this is a a stopgap measure and likely isn't all that useful
#ifndef LIBD_MEM_POOL_INDEX_TYPE
  #define LIBD_MEM_POOL_INDEX_TYPE uint32_t
#endif

#define INDEX_TYPE     LIBD_MEM_POOL_INDEX_TYPE
#define INDEX_TYPE_MAX ((INDEX_TYPE)(-1))
typedef char _index_type_must_be_unsigned[INDEX_TYPE_MAX > 0 ? 1 : -1];

#define POOL_PARAM_NAME     p_allocator
#define INDEX_BYTES(i)      ((i) * POOL_PARAM_NAME->bytes_per_alloc)
#define HEAD_BYTES          (INDEX_BYTES(POOL_PARAM_NAME->head))
#define TAIL_BYTES          (INDEX_BYTES(POOL_PARAM_NAME->tail))
#define POINTER_TO_INDEX(i) (POOL_PARAM_NAME->data + INDEX_BYTES(i))
#define TERMINAL_INDEX      (POOL_PARAM_NAME->max_allocations)

typedef libd_memory_pool_allocator_o pool_allocator;
typedef libd_memory_result_e         result;

struct libd_memory_pool_allocator_s {
  INDEX_TYPE max_allocations, bytes_per_alloc;
  INDEX_TYPE head_index, tail_index;
  uint8_t*   data;
};

result
_initialize_free_list_nodes(pool_allocator* p_allocator);

// NOTE: not used in the code, but here for size and alignment calculations,
// with the possibility of future extension.
typedef struct {
  INDEX_TYPE next_free_slot_index;
} free_node;
static inline int
_aligned_sizeof_free_node()
{
  return libd_memory_align_value(sizeof(free_node), LIBD_ALIGNOF(free_node));
}

result
libd_memory_pool_allocator_create(
  pool_allocator** out_allocator,
  INDEX_TYPE       max_allocations,
  INDEX_TYPE       bytes_per_alloc,
  uint8_t          alignment)
{
  if (out_allocator == NULL) {
    return libd_mem_invalid_null_parameter;
  }
  if (max_allocations == 0 || bytes_per_alloc == 0) {
    return libd_mem_invalid_zero_parameter;
  }
  if (!libd_memory_is_valid_alignment(alignment)) {
    return libd_mem_invalid_alignment;
  }
  // checking that there is room for the terminal index
  if (max_allocations == INDEX_TYPE_MAX) {
    return libd_mem_no_memory;
  }

  uint32_t aligned_bytes_per_alloc =
    libd_memory_align_value(bytes_per_alloc, alignment);

  // Ensure there is enough space for the free list nodes.
  if (aligned_bytes_per_alloc < _aligned_sizeof_free_node()) {
    aligned_bytes_per_alloc = _aligned_sizeof_free_node();
  }

  size_t data_size  = (max_allocations + 1) * aligned_bytes_per_alloc;
  size_t alloc_size = sizeof(pool_allocator) + data_size + alignment - 1;
  pool_allocator* p_allocator = malloc(alloc_size);
  if (p_allocator == NULL) {
    return libd_mem_no_memory;
  }

  p_allocator->max_allocations = max_allocations;
  p_allocator->bytes_per_alloc = aligned_bytes_per_alloc;

  // Aligning the start of data to the given alignment
  uintptr_t aligned_data_start = libd_memory_align_value(
    (uintptr_t)p_allocator + sizeof(pool_allocator), alignment);

  p_allocator->data = (uint8_t*)aligned_data_start;

  if (_initialize_free_list_nodes(p_allocator) != libd_mem_ok) {
    free(p_allocator);
    return libd_mem_init_failure;
  }

  *out_allocator = p_allocator;

  return libd_mem_ok;
}

result
libd_memory_pool_allocator_destroy(pool_allocator* p_allocator)
{
  if (p_allocator == NULL) {
    return libd_mem_invalid_null_parameter;
  }

  free(p_allocator);

  return libd_mem_ok;
}

result
libd_memory_pool_allocator_alloc(
  pool_allocator* p_allocator,
  void**          out_pointer)
{
  if (p_allocator == NULL || out_pointer == NULL) {
    return libd_mem_invalid_null_parameter;
  }

  if (p_allocator->head_index == TERMINAL_INDEX) {
    return libd_mem_no_memory;
  }

  // setting the out pointer to the allocated region.
  *out_pointer = POINTER_TO_INDEX(p_allocator->head_index);

  // copying the next index into head.
  memcpy(&p_allocator->head_index, *out_pointer, sizeof(free_node));

  // If the last slot was just allocated, move the tail as well.
  if (p_allocator->head_index == TERMINAL_INDEX) {
    p_allocator->tail_index = TERMINAL_INDEX;
  }

  return libd_mem_ok;
}

result
libd_memory_pool_allocator_free(
  pool_allocator* p_allocator,
  void*           p_to_free)
{
  if (p_allocator == NULL || p_to_free == NULL) {
    return libd_mem_invalid_null_parameter;
  }

  ptrdiff_t byte_offset = (uint8_t*)p_to_free - p_allocator->data;
  if (byte_offset < 0 || byte_offset % p_allocator->bytes_per_alloc != 0) {
    return libd_mem_invalid_pointer;  // below bounds or not block aligned
  }

  INDEX_TYPE free_index = byte_offset / p_allocator->bytes_per_alloc;
  if (free_index >= p_allocator->max_allocations) {
    return libd_mem_invalid_pointer;  // above bounds
  }

  if (free_index < p_allocator->head_index) {
    // linking the new head to the previous head index.
    memcpy(
      POINTER_TO_INDEX(free_index),
      &p_allocator->head_index,
      sizeof(free_node));

    // updating the head to point at the new index.
    p_allocator->head_index = free_index;
  } else {
    // update the old tail with the index of the newly freed slab.
    memcpy(
      POINTER_TO_INDEX(p_allocator->tail_index),
      &free_index,
      sizeof(free_node));
    // update the tail index
    p_allocator->tail_index = free_index;

    // point the new tail to the terminal index
    INDEX_TYPE temp = TERMINAL_INDEX;
    memcpy(POINTER_TO_INDEX(p_allocator->tail_index), &temp, sizeof(free_node));
  }

  // if we just free'd out of a full allocator, move the tail as well.
  if (p_allocator->tail_index == TERMINAL_INDEX) {
    p_allocator->tail_index = p_allocator->head_index;
  }

  return libd_mem_ok;
}

result
libd_memory_pool_allocator_reset(pool_allocator* p_allocator)
{
  return _initialize_free_list_nodes(p_allocator);
}

result
_initialize_free_list_nodes(pool_allocator* p_allocator)
{
  p_allocator->head_index = 0;
  p_allocator->tail_index = TERMINAL_INDEX - 1;
  INDEX_TYPE next_index;
  for (INDEX_TYPE i = 0; i < p_allocator->tail_index; i += 1) {
    next_index = i + 1;
    memcpy(POINTER_TO_INDEX(i), &next_index, sizeof(free_node));
  }

  next_index = TERMINAL_INDEX;
  memcpy(
    POINTER_TO_INDEX(p_allocator->tail_index), &next_index, sizeof(free_node));

  return libd_mem_ok;
}
