#include "../include/libdane/memory.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

// this is a a stopgap measure and likely isn't all that useful
#ifndef LIBD_MEM_POOL_INDEX_TYPE
  #define LIBD_MEM_POOL_INDEX_TYPE uint32_t
#endif

#define INDEX_TYPE LIBD_MEM_POOL_INDEX_TYPE

#define POOL_PARAM_NAME     p_allocator
#define INDEX_BYTES(i)      ((i) * POOL_PARAM_NAME->bytes_per_alloc)
#define HEAD_BYTES          (INDEX_BYTES(POOL_PARAM_NAME->head))
#define TAIL_BYTES          (INDEX_BYTES(POOL_PARAM_NAME->tail))
#define POINTER_TO_INDEX(i) (POOL_PARAM_NAME->data + INDEX_BYTES(i))
#define TERMINAL_INDEX      (POOL_PARAM_NAME->max_allocations - 1)

typedef libd_memory_pool_allocator_o pool_allocator;
typedef libd_memory_result_e         result;

struct libd_memory_pool_allocator_s {
  INDEX_TYPE max_allocations, bytes_per_alloc;
  INDEX_TYPE head, tail;
  uint8_t*   data;
};

result
_initialize_free_list_nodes(pool_allocator* p_allocator);

// NOTE: not used in the code, but here for size and alignment calculations
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
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }
  if (max_allocations < 2 || bytes_per_alloc == 0) {  // TODO:
    return LIBD_MEM_INVALID_ZERO_PARAMETER;
  }
  if (!libd_memory_is_valid_alignment(alignment)) {
    return LIBD_MEM_INVALID_ALIGNMENT;
  }

  uint32_t aligned_bytes_per_alloc =
    libd_memory_align_value(bytes_per_alloc, alignment);

  // Ensure there is enough space for the free list nodes.
  if (aligned_bytes_per_alloc < _aligned_sizeof_free_node()) {
    aligned_bytes_per_alloc = _aligned_sizeof_free_node();
  }

  size_t data_size  = max_allocations * aligned_bytes_per_alloc;
  size_t alloc_size = sizeof(pool_allocator) + data_size + alignment - 1;
  pool_allocator* p_allocator = malloc(alloc_size);
  if (p_allocator == NULL) {
    return LIBD_MEM_NO_MEMORY;
  }

  p_allocator->max_allocations = max_allocations;
  p_allocator->bytes_per_alloc = aligned_bytes_per_alloc;

  // Aligning the start of data to the given alignment
  uintptr_t aligned_data_start = libd_memory_align_value(
    (uintptr_t)p_allocator + sizeof(pool_allocator), alignment);

  p_allocator->data = (uint8_t*)aligned_data_start;

  // the last index is left open/blank as the terminal index for a full buffer.
  if (_initialize_free_list_nodes(p_allocator) != LIBD_MEM_OK) {
    free(p_allocator);
    return LIBD_MEM_INIT_FAILURE;
  }

  *out_allocator = p_allocator;

  return LIBD_MEM_OK;
}

result
libd_memory_pool_allocator_destroy(pool_allocator* p_allocator)
{
  if (p_allocator == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }

  free(p_allocator);

  return LIBD_MEM_OK;
}

result
libd_memory_pool_allocator_alloc(
  pool_allocator* p_allocator,
  void**          out_pointer)
{
  if (p_allocator == NULL || out_pointer == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }

  if (p_allocator->head == TERMINAL_INDEX) {
    return LIBD_MEM_NO_MEMORY;
  }

  // setting the out pointer to the allocated region.
  *out_pointer = POINTER_TO_INDEX(p_allocator->head);

  // copying the next index into head.
  memcpy(&p_allocator->head, *out_pointer, sizeof(free_node));

  // If the last slot was just allocated, move the tail as well.
  if (p_allocator->head == TERMINAL_INDEX) {
    p_allocator->tail = TERMINAL_INDEX;
  }

  return LIBD_MEM_OK;
}

result
libd_memory_pool_allocator_free(
  pool_allocator* p_allocator,
  void*           p_to_free)
{
  ptrdiff_t byte_offset = (uint8_t*)p_to_free - p_allocator->data;
  if (byte_offset < 0 || byte_offset % p_allocator->bytes_per_alloc != 0) {
    return LIBD_MEM_INVALID_POINTER;  // below bounds or not block aligned
  }

  INDEX_TYPE free_index = byte_offset / p_allocator->bytes_per_alloc;
  if (free_index >= p_allocator->max_allocations) {
    return LIBD_MEM_INVALID_POINTER;  // above bounds
  }

  if (free_index < p_allocator->head) {
    // linking the new head to the previous head index.
    memcpy(POINTER_TO_INDEX(free_index), &p_allocator->head, sizeof(free_node));

    // updating the head to point at the new index.
    p_allocator->head = free_index;
  } else {
    // update the old tail with the index of the newly freed slab.
    memcpy(POINTER_TO_INDEX(p_allocator->tail), &free_index, sizeof(free_node));
    // update the tail index
    p_allocator->tail = free_index;

    // point the new tail to the terminal index
    INDEX_TYPE temp = TERMINAL_INDEX;
    memcpy(POINTER_TO_INDEX(p_allocator->tail), &temp, sizeof(free_node));
  }

  // if we just free'd out of a full allocator, move the tail as well.
  if (p_allocator->tail == TERMINAL_INDEX) {
    p_allocator->tail = p_allocator->head;
  }

  return LIBD_MEM_OK;
}

result
libd_memory_pool_allocator_reset(pool_allocator* p_allocator)
{
  return _initialize_free_list_nodes(p_allocator);
}

result
_initialize_free_list_nodes(pool_allocator* p_allocator)
{
  p_allocator->head = 0;
  p_allocator->tail = TERMINAL_INDEX - 1;
  INDEX_TYPE next_index;
  for (INDEX_TYPE i = 0; i < p_allocator->tail; i += 1) {
    next_index = i + 1;
    memcpy(POINTER_TO_INDEX(i), &next_index, sizeof(free_node));
  }

  next_index = TERMINAL_INDEX;
  memcpy(POINTER_TO_INDEX(p_allocator->tail), &next_index, sizeof(free_node));

  return LIBD_MEM_OK;
}
