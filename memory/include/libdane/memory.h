/**
 * @file memory.h
 * @brief Platform-specific thread local storage interface
 */

#ifndef LIBDANE_MEMORY_H
#define LIBDANE_MEMORY_H

#include <stddef.h>
#include <stdint.h>

//==============================================================================
// Result Codes
//==============================================================================

/**
 * @brief Result codes for memory operations
 */
typedef enum {
  RESULT_OK,                  /**< Operation successful */
  ERR_NO_MEMORY,              /**< No memory for the operation */
  ERR_NOT_IMPLEMENTED,        /**< Functionality not yet implemented */
  ERR_NULL_RECEIVED,          /**< Received a NULL pointer as a parameter */
  LIBD_MEMORY_RESULT_E_COUNT, /**< Count of result states */
} libd_memory_result_e;

//==============================================================================
// Type Definitions
//==============================================================================

/**
 * @brief Opaque handle for the bump arena
 */
typedef struct libd_memory_bump_arena_s libd_memory_bump_arena_s;

/**
 * @brief Opaque handle for the pool allocator
 */
typedef struct libd_memory_pool_arena_s libd_memory_pool_arena_s;

//==============================================================================
// Bump Arena API
//==============================================================================

/**
 * @brief Initializes an arena that holds any types, but can only be reset.
 * @param pp_arena Pointer to recieve the created handle
 * @return RESULT_OK on success, error code otherwise
 */
libd_memory_result_e
libd_memory_bump_arena_create(libd_memory_bump_arena_s** pp_arena,
                              size_t capacity);

/**
 * @brief Sets a given pointer to start of a data block of the provided size.
 * @param p_arena Handle to the desired bump arena
 * @param pp_data Pointer to recieve the created handle
 * @param size Size of the block to allocate for
 * @return RESULT_OK on success, error code otherwise
 */
libd_memory_result_e
libd_memory_bump_arena_alloc(libd_memory_bump_arena_s* p_arena,
                             uint8_t** pp_data,
                             size_t size);

/**
 * @brief Resets the arena. Accessing outdated pointers is undefined
 * @param p_arena Handle to the arena to reset
 * @return RESULT_OK on success, error code otherwise
 */
libd_memory_result_e
libd_memory_bump_arena_reset(libd_memory_bump_arena_s* p_arena);

/**
 * @brief Frees (destroys) the arena
 * @param p_arena Handle to the arena to destroy
 * @return RESULT_OK on success, error code otherwise
 */
libd_memory_result_e
libd_memory_bump_arena_destroy(libd_memory_bump_arena_s* p_arena);


//==============================================================================
// Pool Allocator API
//==============================================================================

/**
 * @brief Initializes an arena with a capacity for objects of a given size
 * @param pp_arena Pointer to recieve the created handle
 * @param capacity Max number of elements the pool should hold
 * @param size Size of the type to be held
 * @param free_capacity Maximum number of elements the free list should be able
 * to reference
 * @return RESULT_OK on success, error code otherwise
 */
libd_memory_result_e
libd_memory_pool_arena_create(libd_memory_pool_arena_s** pp_arena,
                              size_t capacity,
                              size_t datum_size,
                              size_t free_capacity);

/**
 * @brief Allocates space for the chosen type. Typecast the given pointer
 * @param p_arena Handle to the area to use
 * @param pp_data Pointer to the handle for the allocated memory
 * @return RESULT_OK on success, error code otherwise
 */
libd_memory_result_e
libd_memory_pool_arena_alloc(libd_memory_pool_arena_s* p_arena,
                             uint8_t** pp_data);

/**
 * @brief Frees the space of the given object for reuse
 * @param p_arena Handle to the arena
 * @param p_data Handle to the data to free
 * @return RESULT_OK on success, error code otherwise
 */
libd_memory_result_e
libd_memory_pool_arena_free(libd_memory_pool_arena_s* p_arena, uint8_t* p_data);

/**
 * @brief Resets the arena to the empty state
 * @param p_arena Handle to the arena
 */
libd_memory_result_e
libd_memory_pool_arena_reset(libd_memory_pool_arena_s* p_arena);

/**
 * @brief Frees (destroys) the arena
 * @param p_arena Handle to the arena to destroy
 * @return RESULT_OK on success, error code otherwise
 */
libd_memory_result_e
libd_memory_pool_arena_destroy(libd_memory_pool_arena_s* p_arena);

#endif  // LIBDANE_MEMORY_H
