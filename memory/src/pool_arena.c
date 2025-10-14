#include "../include/libdane/memory.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define RBUF_DATA(a) ((uint32_t*)(a)->dat)
#define ARENA_DATA(a) ((a)->dat + (a)->rbuf_capacity * sizeof(uint32_t))

/*
 * The embedded ring buffer is handling the free list.
 */
struct libd_memory_pool_arena_s {
  size_t capacity, size;
  size_t rbuf_capacity, rbuf_read_idx, rbuf_write_idx, rbuf_count;
  uint8_t dat[];
};
typedef struct libd_memory_pool_arena_s pool_arena_s;

static void
_embedded_ring_buffer_init(pool_arena_s* rbuf, size_t capacity);
libd_memory_result_e
_embedded_ring_buffer_read_index(pool_arena_s* p_arena, size_t* next_idx);
libd_memory_result_e
_embedded_ring_buffer_write_index(pool_arena_s* p_arena, size_t freed_index);

libd_memory_result_e
libd_memory_pool_arena_create(pool_arena_s** pp_arena,
                              size_t capacity,
                              size_t datum_size,
                              size_t freelist_capacity)
{
  if (pp_arena == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }
  if (capacity == 0 || datum_size == 0 || freelist_capacity == 0) {
    return ERR_INVALID_ZERO_PARAMETER;
  }

  size_t free_list_size = freelist_capacity * sizeof(uint32_t);
  size_t data_array_size = capacity * datum_size;
  size_t alloc_size = sizeof(pool_arena_s) + free_list_size + data_array_size;

  pool_arena_s* p_arena = malloc(alloc_size);

  if (p_arena == NULL) {
    return ERR_NO_MEMORY;
  }

  p_arena->capacity = capacity;
  p_arena->size = datum_size;


  _embedded_ring_buffer_init(p_arena, freelist_capacity);

  *pp_arena = p_arena;

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_pool_arena_alloc(pool_arena_s* p_arena, uint8_t** pp_data)
{
  size_t next_index = 0;
  if (_embedded_ring_buffer_read_index(p_arena, &next_index) != RESULT_OK) {
    return ERR_NO_MEMORY;
  }
  size_t byte_offset = next_index * p_arena->size;

  *pp_data = ARENA_DATA(p_arena) + byte_offset;

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_pool_arena_free(pool_arena_s* p_arena, uint8_t* p_data)
{
  ptrdiff_t byte_offset = p_data - ARENA_DATA(p_arena);
  if (byte_offset < 0 || byte_offset % p_arena->size != 0) {
    return ERR_INVALID_POINTER;  // below bounds or not aligned
  }

  size_t freed_index = byte_offset / p_arena->size;
  if (freed_index >= p_arena->capacity) {
    return ERR_INVALID_POINTER;  // above bounds
  }

  if (_embedded_ring_buffer_write_index(p_arena, freed_index) != RESULT_OK) {
    // TODO:
    return ERR_NOT_IMPLEMENTED;
  }

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_pool_arena_reset(pool_arena_s* p_arena)
{
  _embedded_ring_buffer_init(p_arena, p_arena->rbuf_capacity);

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_pool_arena_destroy(pool_arena_s* p_arena)
{
  if (p_arena == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }

  free(p_arena);

  return RESULT_OK;
}

static bool
_embedded_rbuf_is_full(pool_arena_s* p_arena);
static bool
_embedded_rbuf_is_empty(pool_arena_s* p_arena);

/*
 * Currently implemented with a counting based state tracking implementation. It
 * is simple, not as performant as it could be. If performance is an issue, look
 * for the floating slot implementation.
 */
static void
_embedded_ring_buffer_init(pool_arena_s* p_arena, size_t rbuf_capacity)
{
  p_arena->rbuf_capacity = rbuf_capacity;
  p_arena->rbuf_write_idx = 0;
  p_arena->rbuf_read_idx = 0;
  p_arena->rbuf_count = rbuf_capacity;

  uint32_t* index_data = RBUF_DATA(p_arena);

  for (size_t i = 0; i < rbuf_capacity; i++) {
    index_data[i] = i;
  }
}

libd_memory_result_e
_embedded_ring_buffer_read_index(pool_arena_s* p_arena, size_t* next_idx)
{
  if (_embedded_rbuf_is_empty(p_arena)) {
    return ERR_NO_MEMORY;  // Freelist is empty so the arena is full.
  }

  *next_idx = RBUF_DATA(p_arena)[p_arena->rbuf_read_idx];

  p_arena->rbuf_read_idx =
    (p_arena->rbuf_read_idx + 1) % p_arena->rbuf_capacity;
  p_arena->rbuf_count -= 1;

  return RESULT_OK;
}

libd_memory_result_e
_embedded_ring_buffer_write_index(pool_arena_s* p_arena, size_t freed_index)
{
  if (_embedded_rbuf_is_full(p_arena)) {
    return ERR_INVALID_FREE;  // Freelist is full, so the arena is empty.
  }

  RBUF_DATA(p_arena)[p_arena->rbuf_write_idx] = freed_index;

  p_arena->rbuf_write_idx =
    (p_arena->rbuf_write_idx + 1) % p_arena->rbuf_capacity;
  p_arena->rbuf_count += 1;

  return RESULT_OK;
}

static bool
_embedded_rbuf_is_full(pool_arena_s* p_arena)
{
  return p_arena->rbuf_count == p_arena->rbuf_capacity;
}
static bool
_embedded_rbuf_is_empty(pool_arena_s* p_arena)
{
  return p_arena->rbuf_count == 0;
}
