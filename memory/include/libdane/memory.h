#ifndef LIBDANE_MEMORY_H
#define LIBDANE_MEMORY_H

#include <stddef.h>
#include <stdint.h>

typedef enum {
  RESULT_OK,
  ERR_NO_MEMORY,
  ERR_NOT_IMPLEMENTED,
  LIBD_MEMORY_RESULT_E_COUNT,
} libd_memory_result_e;

typedef struct libd_memory_bump_arena_s libd_memory_bump_arena_s;

/*
 * Initializes an arena that holds any types, but can only be reset.
 */
libd_memory_result_e libd_memory_bump_arena_create(libd_memory_bump_arena_s** pp_arena, size_t capacity);

/*
 * Sets a given pointer to start of a data block of the provided size.
 */
libd_memory_result_e libd_memory_bump_arena_alloc(libd_memory_bump_arena_s* p_arena, uint8_t** pp_data, size_t size);

/*
 * Resets the arena. Any old memory accesses after a reset are undefined.
 */
void libd_memory_bump_arena_reset(libd_memory_bump_arena_s* p_arena);

typedef struct libd_memory_monotyped_arena_s libd_memory_monotyped_arena_s;

/*
 * Initializes an arena with a capacity for objects of the given size
 */
libd_memory_result_e libd_memory_monotype_arena_create(libd_memory_monotyped_arena_s** pp_arena, size_t capacity, size_t free_capacity,
                                                       size_t datum_size);

/*
 * Cast the returned pointer to the necessary type
 */
libd_memory_result_e libd_memory_monotype_arena_alloc(libd_memory_monotyped_arena_s* p_arena, uint8_t** pp_data);

/*
 * Provide a pointer to the data for the arena to free
 */
libd_memory_result_e libd_memory_monotype_arena_free(libd_memory_monotyped_arena_s* p_arena, uint8_t* data);

/*
 * Cleans out all of the stored data and restores a fresh state
 */
void libd_memory_monotype_arena_reset(libd_memory_monotyped_arena_s* p_arena);

#endif // LIBDANE_MEMORY_H
