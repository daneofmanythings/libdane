#include "../include/libdane/memory.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define FREE_LIST(a)  ((uint32_t*)(a)->dat)
#define DATA_ARRAY(a) ((a)->dat + (a)->rbuf_capacity * sizeof(uint32_t))

/*
 * The embedded ring buffer is handling the free list.
 */
struct libd_memory_slab_allocator_s {
  size_t  max_allocations, bytes_per_alloc;
  size_t  rbuf_capacity, rbuf_read_idx, rbuf_write_idx, rbuf_count;
  uint8_t dat[];
};
typedef libd_memory_slab_allocator_o slab_allocator;

static void
_embedded_ring_buffer_init(
  slab_allocator* rbuf,
  size_t          capacity);

libd_memory_result_e
_embedded_ring_buffer_read_index(
  slab_allocator* p_arena,
  size_t*         next_idx);

libd_memory_result_e
_embedded_ring_buffer_write_index(
  slab_allocator* p_arena,
  size_t          freed_index);

libd_memory_result_e
libd_memory_slab_allocator_create(
  slab_allocator** out_allocator,
  size_t           max_allocations,
  size_t           bytes_per_alloc,
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

  size_t aligned_bytes_per_alloc =
    libd_memory_align_value(bytes_per_alloc, alignment);

  size_t free_list_size_bytes  = max_allocations * sizeof(uint32_t);
  size_t data_array_size_bytes = max_allocations * aligned_bytes_per_alloc;
  size_t total_allocation_size_bytes =
    sizeof(slab_allocator) + free_list_size_bytes + data_array_size_bytes;

  slab_allocator* p_allocator = malloc(total_allocation_size_bytes);

  if (p_allocator == NULL) {
    return LIBD_MEM_NO_MEMORY;
  }

  p_allocator->max_allocations = max_allocations;
  p_allocator->bytes_per_alloc = aligned_bytes_per_alloc;

  _embedded_ring_buffer_init(p_allocator, max_allocations);

  *out_allocator = p_allocator;

  return LIBD_MEM_OK;
}

libd_memory_result_e
libd_memory_slab_allocator_alloc(
  slab_allocator* p_allocator,
  void**          out_pointer)
{
  size_t next_index = 0;
  if (
    _embedded_ring_buffer_read_index(p_allocator, &next_index) != LIBD_MEM_OK) {
    return LIBD_MEM_NO_MEMORY;
  }
  size_t byte_offset = next_index * p_allocator->bytes_per_alloc;

  *out_pointer = DATA_ARRAY(p_allocator) + byte_offset;

  return LIBD_MEM_OK;
}

libd_memory_result_e
libd_memory_slab_allocator_free(
  slab_allocator* p_allocator,
  void*           p_to_free)
{
  ptrdiff_t byte_offset = (uint8_t*)p_to_free - DATA_ARRAY(p_allocator);
  if (byte_offset < 0 || byte_offset % p_allocator->bytes_per_alloc != 0) {
    return LIBD_MEM_INVALID_POINTER;  // below bounds or not block aligned
  }

  size_t freed_index = byte_offset / p_allocator->bytes_per_alloc;
  if (freed_index >= p_allocator->max_allocations) {
    return LIBD_MEM_INVALID_POINTER;  // above bounds
  }

  libd_memory_result_e result =
    _embedded_ring_buffer_write_index(p_allocator, freed_index);
  if (result != LIBD_MEM_OK) {
    return result;
  }

  return LIBD_MEM_OK;
}

libd_memory_result_e
libd_memory_slab_allocator_reset(slab_allocator* p_allocator)
{
  _embedded_ring_buffer_init(p_allocator, p_allocator->rbuf_capacity);

  return LIBD_MEM_OK;
}

libd_memory_result_e
libd_memory_slab_allocator_destroy(slab_allocator* p_allocator)
{
  if (p_allocator == NULL) {
    return LIBD_MEM_INVALID_NULL_PARAMETER;
  }

  free(p_allocator);

  return LIBD_MEM_OK;
}

static bool
_embedded_rbuf_is_full(slab_allocator* p_allocator);
static bool
_embedded_rbuf_is_empty(slab_allocator* p_allocator);

/*
 * Currently implemented with a counting based state tracking. It is simple, not
 * as performant as it could be. If performance is an issue, look for the
 * floating slot implementation.
 */
static void
_embedded_ring_buffer_init(
  slab_allocator* p_allocator,
  size_t          rbuf_capacity)
{
  p_allocator->rbuf_capacity  = rbuf_capacity;
  p_allocator->rbuf_write_idx = 0;
  p_allocator->rbuf_read_idx  = 0;

  // Seeding the ring buffer with every index because an empty arena has all
  // slots availible.
  p_allocator->rbuf_count = rbuf_capacity;
  for (size_t i = 0; i < rbuf_capacity; i++) {
    FREE_LIST(p_allocator)[i] = i;
  }
}

libd_memory_result_e
_embedded_ring_buffer_read_index(
  slab_allocator* p_allocator,
  size_t*         next_idx)
{
  if (_embedded_rbuf_is_empty(p_allocator)) {
    return LIBD_MEM_NO_MEMORY;  // Freelist is empty so the arena is full.
  }

  *next_idx = FREE_LIST(p_allocator)[p_allocator->rbuf_read_idx];

  p_allocator->rbuf_read_idx =
    (p_allocator->rbuf_read_idx + 1) % p_allocator->rbuf_capacity;
  p_allocator->rbuf_count -= 1;

  return LIBD_MEM_OK;
}

libd_memory_result_e
_embedded_ring_buffer_write_index(
  slab_allocator* p_allocator,
  size_t          freed_index)
{
  if (_embedded_rbuf_is_full(p_allocator)) {
    return LIBD_MEM_INVALID_FREE;  // Freelist is full, so the arena is empty.
  }

  FREE_LIST(p_allocator)[p_allocator->rbuf_write_idx] = freed_index;

  p_allocator->rbuf_write_idx =
    (p_allocator->rbuf_write_idx + 1) % p_allocator->rbuf_capacity;
  p_allocator->rbuf_count += 1;

  return LIBD_MEM_OK;
}

static bool
_embedded_rbuf_is_full(slab_allocator* p_allocator)
{
  return p_allocator->rbuf_count == p_allocator->rbuf_capacity;
}
static bool
_embedded_rbuf_is_empty(slab_allocator* p_allocator)
{
  return p_allocator->rbuf_count == 0;
}
