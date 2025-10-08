#include "../include/libdane/memory.h"
#include "internal/containers.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define EMBEDDED_RBUF(a) (index_ring_buffer_t*)&(a)->free_capacity

struct libd_memory_monotyped_arena_s {
  size_t capacity, size;
  size_t free_capacity, free_head, free_tail;
  uint8_t dat[];
};
typedef struct libd_memory_monotyped_arena_s monotyped_arena_s;

libd_memory_result_e libd_memory_monotype_arena_create(monotyped_arena_s** pp_arena, size_t capacity, size_t free_capacity, size_t datum_size) {
  size_t free_list_size = free_capacity * sizeof(uint32_t);
  size_t data_array_size = capacity * datum_size;
  monotyped_arena_s* p_arena = (monotyped_arena_s*)malloc(sizeof(monotyped_arena_s) + free_list_size + data_array_size);
  if (p_arena == NULL) {
    // TODO: integrate errors to handle this case
    return ERR_NOMEM;
  }

  p_arena->capacity = capacity;
  p_arena->size = datum_size;

  index_ring_buffer_t* rbuf = EMBEDDED_RBUF(p_arena);
  index_ring_buffer_init(rbuf, free_capacity);

  *pp_arena = p_arena;

  return OK;
}

libd_memory_result_e libd_memory_monotype_arena_alloc(monotyped_arena_s* p_arena, uint8_t** pp_data) {
  // TODO: get the next open index

  // TODO: get the pointer to the spot in the data array

  // TODO: set the out pointer

  return ERR_NOT_IMPLEMENTED;
}

libd_memory_result_e libd_memory_monotype_arena_free(monotyped_arena_s* arena, uint8_t* data) {
  uint8_t index = (data - arena->dat) / arena->size;
  // TODO: check that the index is in-bounds

  // TODO: place the index at the free_tail and increment it

  return ERR_NOT_IMPLEMENTED;
}

void libd_memory_monotype_arena_reset(monotyped_arena_s* arena) {
  // TODO: reset the free_list
}
