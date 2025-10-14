#include "../include/libdane/memory.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

struct libd_memory_stack_arena_s {
  size_t capacity, size;
  size_t stack_capacity, stack_count;
  uint8_t dat[];
};

#define ARENA_STACK(a) ((size_t*)(a)->dat)
#define ARENA_DATA(a) ((a)->dat + (a)->stack_capacity * sizeof(size_t))

static libd_memory_result_e
_embedded_stack_init(libd_memory_stack_arena_s* p_arena, size_t capacity);

static libd_memory_result_e
_embedded_stack_push(libd_memory_stack_arena_s* p_arena, size_t size_bytes);

static libd_memory_result_e
_embedded_stack_pop(libd_memory_stack_arena_s* p_arena, size_t* bytes_freed);

libd_memory_result_e
libd_memory_stack_arena_create(libd_memory_stack_arena_s** pp_arena,
                               size_t capacity_bytes,
                               size_t stack_capacity)
{
  if (pp_arena == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }
  if (capacity_bytes == 0 || stack_capacity == 0) {
    return ERR_INVALID_ZERO_PARAMETER;
  }

  size_t stack_capacity_bytes = stack_capacity * sizeof(size_t);
  size_t arena_capacity_bytes = capacity_bytes;
  size_t alloc_size_bytes = stack_capacity_bytes + arena_capacity_bytes;

  libd_memory_stack_arena_s* p_arena = malloc(alloc_size_bytes);
  if (p_arena == NULL) {
    return ERR_NO_MEMORY;
  }

  p_arena->capacity = capacity_bytes;
  p_arena->size = 0;

  if (_embedded_stack_init(p_arena, stack_capacity) != RESULT_OK) {
    // TODO:
  }

  *pp_arena = p_arena;

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_stack_arena_destroy(libd_memory_stack_arena_s* p_arena)
{
  if (p_arena == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }

  free(p_arena);

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_stack_arena_alloc(libd_memory_stack_arena_s* p_arena,
                              uint8_t** p_handle,
                              size_t size_bytes)
{
  if (p_arena == NULL || p_handle == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }
  if (size_bytes == 0) {
    return ERR_INVALID_ZERO_PARAMETER;
  }
  if (p_arena->size + size_bytes > p_arena->capacity) {
    return ERR_NO_MEMORY;
  }


  libd_memory_result_e result = _embedded_stack_push(p_arena, size_bytes);
  if (result != RESULT_OK) {
    return result;
  }

  *p_handle = ARENA_DATA(p_arena) + p_arena->size;
  p_arena->size += size_bytes;

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_stack_arena_free(libd_memory_stack_arena_s* p_arena)
{
  if (p_arena == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }

  size_t bytes_freed = 0;
  if (_embedded_stack_pop(p_arena, &bytes_freed) != RESULT_OK) {
    return ERR_STACK_EMPTY;
  }

  p_arena->size -= bytes_freed;

  return RESULT_OK;
}

libd_memory_result_e
libd_memory_stack_arena_reset(libd_memory_stack_arena_s* p_arena)
{
  if (p_arena == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }

  p_arena->size = 0;
  _embedded_stack_init(p_arena, p_arena->stack_capacity);

  return RESULT_OK;
}

static libd_memory_result_e
_embedded_stack_init(libd_memory_stack_arena_s* p_arena, size_t capacity)
{
  if (p_arena == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }

  p_arena->stack_capacity = capacity;
  p_arena->stack_count = 0;

  return RESULT_OK;
}

static libd_memory_result_e
_embedded_stack_push(libd_memory_stack_arena_s* p_arena, size_t size_bytes)
{
  if (p_arena == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }
  if (size_bytes == 0) {
    return ERR_INVALID_ZERO_PARAMETER;
  }

  if (p_arena->stack_count >= p_arena->stack_capacity) {
    return ERR_STACK_FULL;
  }

  ARENA_STACK(p_arena)[p_arena->stack_count] = size_bytes;
  p_arena->stack_count += 1;

  return RESULT_OK;
}

static libd_memory_result_e
_embedded_stack_pop(libd_memory_stack_arena_s* p_arena, size_t* bytes_freed)
{
  if (p_arena == NULL) {
    return ERR_INVALID_NULL_PARAMETER;
  }
  if (p_arena->stack_count == 0) {
    return ERR_STACK_EMPTY;
  }

  *bytes_freed = ARENA_STACK(p_arena)[p_arena->stack_count];
  p_arena->stack_count -= 1;

  return RESULT_OK;
}
