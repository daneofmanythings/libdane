#include "../include/libdane/memory.h"
#include <stdint.h>
#include <stdlib.h>

struct libd_memory_bump_arena_s {
  size_t capacity;
  size_t cursor_index;
  uint8_t dat[];
};
typedef libd_memory_bump_arena_s bump_arena_s;

libd_memory_result_e
libd_memory_bump_arena_create(bump_arena_s** pp_arena, size_t capacity)
{
  if (pp_arena == NULL) {
    return ERR_NULL_RECEIVED;
  }

  size_t alloc_size = sizeof(bump_arena_s) + capacity + sizeof(uint8_t);
  bump_arena_s* p_arena = (bump_arena_s*)malloc(alloc_size);
  if (p_arena == NULL) {
    // TODO: this is a fatal error
    return ERR_NO_MEMORY;
  }

  p_arena->capacity = capacity;
  p_arena->cursor_index = 0;

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_bump_arena_alloc(bump_arena_s* p_arena,
                             uint8_t** pp_data,
                             size_t size)
{
  if (p_arena == NULL || pp_data == NULL) {
    return ERR_NULL_RECEIVED;
  }

  if (p_arena->capacity < p_arena->cursor_index + size) {
    return ERR_NO_MEMORY;
  }
  *pp_data = &p_arena->dat[p_arena->cursor_index];
  p_arena->cursor_index += size;

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_bump_arena_reset(bump_arena_s* p_arena)
{
  if (p_arena == NULL) {
    return ERR_NULL_RECEIVED;
  }

  p_arena->cursor_index = 0;

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_bump_arena_destroy(bump_arena_s* p_arena)
{
  if (p_arena == NULL) {
    return ERR_NULL_RECEIVED;
  }

  free(p_arena);

  return RESULT_OK;
}
