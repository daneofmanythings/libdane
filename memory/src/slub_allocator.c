#include "../include/libdane/memory.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define VALUE_AT_INDEX(i)                                 \
  (p_allocator->data[(i) * p_allocator->bytes_per_alloc])

typedef libd_memory_slub_allocator_o slub_allocator;
typedef libd_memory_result_e         result;

struct libd_memory_slub_allocator_s {
  uint32_t max_allocations, bytes_per_alloc;
  uint32_t head, tail;
  uint8_t  data[];
};

result
_initialize_free_list_nodes(slub_allocator* p_allocator);

static inline void
_insert_value_at_index(
  slub_allocator* p_allocator,
  uint32_t        value,
  uint32_t        index);

typedef struct {
  uint32_t free_slot_index;
  uint32_t next_node_index;
} free_node;
#define ALIGNED_SIZEOF_FREE_NODE                                      \
  libd_memory_align_value(sizeof(free_node), LIBD_ALIGNOF(free_node))

result
libd_memory_slub_allocator_create(
  slub_allocator** out_allocator,
  uint32_t         max_allocations,
  uint32_t         bytes_per_alloc,
  uint8_t          alignment)
{
  if (out_allocator == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }
  if (max_allocations == 0 || bytes_per_alloc == 0) {
    return LIBD_MEM_INVALID_ZERO_PARAMETER;
  }
  if (!libd_memory_is_valid_alignment(alignment)) {
    return LIBD_MEM_INVALID_ALIGNMENT;
  }

  uint32_t aligned_bytes_per_alloc =
    libd_memory_align_value(bytes_per_alloc, alignment);

  // Ensure there is enough space for the free list nodes.
  if (aligned_bytes_per_alloc > ALIGNED_SIZEOF_FREE_NODE) {
    aligned_bytes_per_alloc = ALIGNED_SIZEOF_FREE_NODE;
  }

  uint32_t data_size = max_allocations * aligned_bytes_per_alloc;

  slub_allocator* p_allocator = malloc(sizeof(slub_allocator) + data_size);
  if (p_allocator == NULL) {
    return LIBD_MEM_NO_MEMORY;
  }

  p_allocator->max_allocations = max_allocations;
  p_allocator->bytes_per_alloc = bytes_per_alloc;

  // the last index is left open/blank as the terminal index for a full buffer.
  _initialize_free_list_nodes(p_allocator);
  return LIBD_MEM_OK;
}

result
libd_memory_slub_allocator_destroy(slub_allocator* p_allocator)
{
  if (p_allocator == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }

  free(p_allocator);

  return LIBD_MEM_OK;
}

result
libd_memory_slub_allocator_alloc(
  slub_allocator* p_allocator,
  void**          out_pointer)
{
  if (p_allocator == NULL || out_pointer == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }

  *out_pointer      = &p_allocator->data[INDEX_OF(p_allocator->head)];
  p_allocator->head = *(uint32_t*)*out_pointer;

  return LIBD_MEM_OK;
}

result
libd_memory_slub_allocator_free(
  slub_allocator* p_allocator,
  void*           p_to_free)
{
  ptrdiff_t byte_offset = (uint8_t*)p_to_free - p_allocator->data;
  if (byte_offset < 0 || byte_offset % p_allocator->bytes_per_alloc != 0) {
    return LIBD_MEM_INVALID_POINTER;  // below bounds or not block aligned
  }

  size_t free_index = byte_offset / p_allocator->bytes_per_alloc;
  if (free_index >= p_allocator->max_allocations) {
    return LIBD_MEM_INVALID_POINTER;  // above bounds
  }

  uint32_t temp = 0;
  if (free_index < p_allocator->head) {
    temp = p_allocator->data[p_allocator->head];
    memcpy(&temp, &p_allocator->data[p_allocator->head], sizeof(temp));

    // Assigning a new head
    p_allocator->head = free_index;

    // pointing to the old head
    _insert_value_at_index(p_allocator, temp, free_index);
  } else {
  }

  return LIBD_MEM_OK;
}

result
libd_memory_slub_allocator_reset(slub_allocator* p_allocator)
{
  return _initialize_free_list_nodes(p_allocator);
}

result
_initialize_free_list_nodes(slub_allocator* p_allocator)
{
  p_allocator->head = 0;
  p_allocator->tail = (p_allocator->max_allocations - 1);
  uint32_t index_to_copy;
  for (uint32_t i = 0; i < p_allocator->tail; i += 1) {
    _insert_value_at_index(p_allocator, i + 1, i);
  }

  return LIBD_MEM_OK;
}

static inline void
_insert_value_at_index(
  slub_allocator* p_allocator,
  uint32_t        value,
  uint32_t        index)
{
  memcpy(
    &p_allocator->data[index * p_allocator->bytes_per_alloc],
    &value,
    sizeof(value));
}
