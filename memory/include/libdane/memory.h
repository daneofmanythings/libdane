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
 * @brief Result codes for memory operations.
 */
typedef enum {
  RESULT_OK,                  /**< Operation successful */
  ERR_NO_MEMORY,              /**< No memory for the operation */
  ERR_NOT_IMPLEMENTED,        /**< Functionality not yet implemented */
  ERR_INVALID_NULL_PARAMETER, /**< A NULL pointer parameter was detected */
  ERR_INVALID_POINTER,        /**< An invalid pointer parameter was detected */
  ERR_INVALID_ZERO_PARAMETER, /**< An invalid zero value as a parameter */
  ERR_INVALID_FREE, /**< Tried to free memory from an arena that has no
                       allocations */
  ERR_STACK_FULL,   /**< Tried to allocate when the stack is full */
  ERR_STACK_EMPTY,  /**< Tried to free when the stack is empty */
  LIBD_MEMORY_RESULT_E_COUNT, /**< Count of result states */
} libd_memory_result_e;

//==============================================================================
// Type Definitions
//==============================================================================

/**
 * @brief Opaque handle for the bump allocator.
 */
typedef struct libd_memory_bump_arena_s libd_memory_bump_arena_s;

/**
 * @brief Opaque handle for the pool allocator.
 */
typedef struct libd_memory_pool_arena_s libd_memory_pool_arena_s;

/**
 * @brief Opque handle to the stack allocator.
 */
typedef struct libd_memory_stack_arena_s libd_memory_stack_arena_s;

//==============================================================================
// Bump Arena API
//==============================================================================

/**
 * @brief Creates a general purpose arena.
 * @param pp_arena Out parameter for the created arena.
 * @param capacity_bytes Capacity for the arena in bytes.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_bump_arena_create(libd_memory_bump_arena_s** pp_arena,
                              size_t capacity_bytes);

/**
 * @brief Allocates memory in the arena.
 * @param p_arena Handle to the arena.
 * @param pp_data Out parameter for the pointer to the allocation.
 * @param size_bytes Size in bytes for the allocation.
 * @return RESULT_OK on success, error code otherwise.
 */
libd_memory_result_e
libd_memory_bump_arena_alloc(libd_memory_bump_arena_s* p_arena,
                             uint8_t** pp_data,
                             size_t size_bytes);

/**
 * @brief Resets the arena by freeing all memory.
 * @param p_arena Handle to the arena.
 * @return RESULT_OK on success, error code otherwise.
 */
libd_memory_result_e
libd_memory_bump_arena_reset(libd_memory_bump_arena_s* p_arena);

/**
 * @brief Destroys the arena.
 * @param p_arena Handle for the arena.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_bump_arena_destroy(libd_memory_bump_arena_s* p_arena);


//==============================================================================
// Pool Allocator API
//==============================================================================

/**
 * @brief Creates an arena for allocations of a fixed size.
 * @warning Using a freelist capacity of 0 is treated as an error. Use a bump
 * arena instead.
 * @param pp_arena Out parameter for the created arena.
 * @param capacity Maximum number of allocations the arena will handle.
 * @param size Size of each allocation in bytes.
 * @param free_capacity Maximum number of deallocations that will be tracked.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_pool_arena_create(libd_memory_pool_arena_s** pp_arena,
                              size_t capacity,
                              size_t datum_size,
                              size_t free_capacity);

/**
 * @brief Destroys the arena.
 * @param p_arena Handle for the arena.
 * @return RESULT_OK on success, error code otherwise
 */
libd_memory_result_e
libd_memory_pool_arena_destroy(libd_memory_pool_arena_s* p_arena);

/**
 * @brief Allocates memory in the arena.
 * @param p_arena Handle for the area.
 * @param pp_data Out parameter for the pointer to the allocation.
 * @return RESULT_OK on success, error code otherwise.
 */
libd_memory_result_e
libd_memory_pool_arena_alloc(libd_memory_pool_arena_s* p_arena,
                             uint8_t** pp_data);

/**
 * @brief Frees the allocation for the given handle.
 * @param p_arena Handle for the arena.
 * @param p_data Handle for the allocation to free.
 * @return RESULT_OK on success, error code otherwise.
 */
libd_memory_result_e
libd_memory_pool_arena_free(libd_memory_pool_arena_s* p_arena, uint8_t* p_data);

/**
 * @brief Resets the arena, freeing all allocations.
 * @param p_arena Handle for the arena.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_pool_arena_reset(libd_memory_pool_arena_s* p_arena);

//==============================================================================
// Stack Allocator API
//==============================================================================

/**
 * @brief Creates an arena which behaves like a stack.
 * @param pp_arena Out parameter for the arena.
 * @param capacity_bytes The capacity of the arena in bytes.
 * @param stack_capacity Maximum allocations the stack will track.
 * @return RESULT_OK if success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_stack_arena_create(libd_memory_stack_arena_s** pp_arena,
                               size_t capacity_bytes,
                               size_t stack_capacity);

/**
 * @brief Destroys the arena.
 * @param p_arena Handle for the arena.
 * @return RESULT_OK if success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_stack_arena_destroy(libd_memory_stack_arena_s* p_arena);

/**
 * @brief Allocates memory in the arena.
 * @param p_arena Handle to the arena.
 * @param p_handle Out parameter for pointer to the allocation.
 * @param size_bytes Number of bytes to allocate.
 * @return RESULT_OK if success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_stack_arena_alloc(libd_memory_stack_arena_s* p_arena,
                              uint8_t** p_handle,
                              size_t size_bytes);

/**
 * @brief Frees the top of the stack.
 * @param p_arena Handle to the arena.
 * @return RESULT_OK if success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_stack_arena_free(libd_memory_stack_arena_s* p_arena);

/**
 * @brief Resets the arena, freeing all memory.
 * @param p_arena Handle to the arena.
 */
libd_memory_result_e
libd_memory_stack_arena_reset(libd_memory_stack_arena_s* p_arena);

#endif  // LIBDANE_MEMORY_H
