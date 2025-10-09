#include "../include/libdane/memory.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define RBUF_INDICES(a) (uint32_t*)(a)->dat
#define ARENA_DATA(a) (a)->dat + (a)->rbuf_capacity * sizeof(uint32_t)

struct libd_memory_pool_arena_s {
  size_t capacity, size;
  size_t rbuf_capacity, rbuf_head, rbuf_tail;
  uint8_t dat[];
};
typedef struct libd_memory_pool_arena_s pool_arena_s;

static void
_embedded_index_ring_buffer_init(pool_arena_s* rbuf, size_t capacity);

libd_memory_result_e
libd_memory_pool_arena_create(pool_arena_s** pp_arena,
                              size_t capacity,
                              size_t datum_size,
                              size_t freelist_capacity)
{
  if (pp_arena == NULL) {
    return ERR_NULL_RECEIVED;
  }

  size_t free_list_size = freelist_capacity * sizeof(uint32_t);
  size_t data_array_size = capacity * datum_size;
  size_t alloc_size = sizeof(pool_arena_s) + free_list_size + data_array_size;

  pool_arena_s* p_arena = (pool_arena_s*)malloc(alloc_size);

  if (p_arena == NULL) {
    // TODO: integrate errors to handle this case
    return ERR_NO_MEMORY;
  }

  p_arena->capacity = capacity;
  p_arena->size = datum_size;


  _embedded_index_ring_buffer_init(p_arena, freelist_capacity);

  *pp_arena = p_arena;

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_pool_arena_alloc(pool_arena_s* p_arena, uint8_t** pp_data)
{
  // TODO: get the next open index

  // TODO: get the pointer to the spot in the data array

  // TODO: set the out pointer

  return ERR_NOT_IMPLEMENTED;
}

libd_memory_result_e
libd_memory_pool_arena_free(pool_arena_s* p_arena, uint8_t* p_data)
{
  uint8_t index = (p_data - p_arena->dat) / p_arena->size;
  // TODO: check that the index is in-bounds

  // TODO: place the index at the free_tail and increment it

  return ERR_NOT_IMPLEMENTED;
}

libd_memory_result_e
libd_memory_pool_arena_reset(pool_arena_s* p_arena)
{
  // TODO: reset the free_list

  return ERR_NOT_IMPLEMENTED;
}

libd_memory_result_e
libd_memory_pool_arena_destroy(pool_arena_s* p_arena)
{
  if (p_arena == NULL) {
    return ERR_NULL_RECEIVED;
  }

  free(p_arena);

  return RESULT_OK;
}

static void
_embedded_index_ring_buffer_init(pool_arena_s* p_arena, size_t rbuf_capacity)
{
  p_arena->rbuf_capacity = rbuf_capacity;
  p_arena->rbuf_tail = 0;
  p_arena->rbuf_head = 0;

  uint32_t* indices = RBUF_INDICES(p_arena);

  for (size_t i = 0; i < rbuf_capacity; i++) {
    indices[i] = i;
  }
}
