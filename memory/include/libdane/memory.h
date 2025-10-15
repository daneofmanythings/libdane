/**
 * @file memory.h
 * @brief Platform-specific thread local storage interface
 */

#ifndef LIBDANE_MEMORY_H
#define LIBDANE_MEMORY_H

#include <stdbool.h>
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
  ERR_INVALID_ALIGNMENT,      /**< An invalid alignment value was given */
  ERR_INVALID_NULL_PARAMETER, /**< A NULL pointer parameter was detected */
  ERR_INVALID_POINTER,        /**< An invalid pointer parameter was detected */
  ERR_INVALID_ZERO_PARAMETER, /**< An invalid zero value as a parameter */
  ERR_INVALID_FREE, /**< Tried to free memory from an arena that has no
                       allocations */
  LIBD_MEMORY_RESULT_E_COUNT, /**< Count of result states */
} libd_memory_result_e;

//==============================================================================
// Type Definitions
//==============================================================================

/**
 * @brief Opaque handle for the linear allocator.
 */
typedef struct libd_memory_linear_allocator_s libd_memory_linear_allocator_s;

/**
 * @brief Opaque handle to a linear allocator checkpoint;
 */
typedef struct libd_memory_linear_allocator_savepoint_s
  libd_memory_linear_allocator_savepoint_s;

/**
 * @brief Opaque handle for the pool allocator.
 */
typedef struct libd_memory_pool_allocator_s libd_memory_pool_allocator_s;

//==============================================================================
// Inline Helper Functions
//==============================================================================

/**
 * @brief Rounds the offset to the nearest given alignment value.
 * @warning alignment must be non-zero.
 * @param offset The value to round from.
 * @param alignment The power of 2 to round to. This parameter MUST be a power
 * of 2.
 */
static inline size_t
libd_memory_align_offset(size_t offset, uint8_t alignment)
{
  return ((offset + alignment - 1) & ~(alignment - 1));
}

/**
 * @brief Check whether or not the given offset is a power of 2.
 * @param The offset to validate.
 * @return true if power of 2, false otherwise.
 */
static inline bool
libd_memory_is_power_of_two(size_t offset)
{
  return ((offset & (offset - 1)) == 0);
}

//==============================================================================
// Linear Allocator API
//==============================================================================

/**
 * @brief Creates a linear allocator.
 * @param out_allocator Out parameter for the allocator.
 * @param capacity_bytes Capacity for the arena in bytes.
 * @param alignment Alignment value for the allocator. Must be a power of 2.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_linear_allocator_create(
  libd_memory_linear_allocator_s** out_allocator,
  size_t capacity_bytes,
  uint8_t alignment);

/**
 * @brief Destroys the allocator.
 * @param p_allocator Handle for the arena.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_linear_allocator_destroy(
  libd_memory_linear_allocator_s* p_allocator);

/**
 * @brief Allocates memory in the arena.
 * @param p_allocator Handle to the arena.
 * @param out_pointer Out parameter for the pointer to the allocation.
 * @param size_bytes Size in bytes for the allocation.
 * @return RESULT_OK on success, error code otherwise.
 */
libd_memory_result_e
libd_memory_linear_allocator_alloc(libd_memory_linear_allocator_s* p_allocator,
                                   void** out_pointer,
                                   size_t size_bytes);

/**
 * @brief Sets a savepoint which can be restored to.
 * @param p_allocator Handle to the allocator.
 * @param out_savepoint Out parameter for the savepoint.
 * @returns RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_linear_allocator_set_savepoint(
  const libd_memory_linear_allocator_s* p_allocator,
  libd_memory_linear_allocator_savepoint_s* out_savepoint);

/**
 * @brief Restores the allocator state to when the savepoint was created by
 * freeing all allocations performed after the savepoint.
 * @warning Restoring the allocator to a savepoint invalidates all savepoints
 * created after it. Attempting to restore to a savepoint that has been
 * invalidated is undefined behavior.
 * @param p_allocator Handle to the allocator.
 * @param p_savepoint Handle to the savepoint.
 * @returns RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_linear_allocator_restore_savepoint(
  libd_memory_linear_allocator_s* p_allocator,
  const libd_memory_linear_allocator_savepoint_s* p_savepoint);

/**
 * @brief Resets the allocator to the default state by freeing all allocations.
 * @param p_allocator Handle to the allocator.
 * @return RESULT_OK on success, error code otherwise.
 */
libd_memory_result_e
libd_memory_linear_allocator_reset(libd_memory_linear_allocator_s* p_allocator);


//==============================================================================
// Pool Allocator API
//==============================================================================

/**
 * @brief Creates an arena for allocations of a fixed size.
 * @warning Using a freelist capacity of 0 is treated as an error. Use a linear
 * arena instead.
 * @param pp_arena Out parameter for the created arena.
 * @param capacity Maximum number of allocations the arena will handle.
 * @param size Size of each allocation in bytes.
 * @param free_capacity Maximum number of deallocations that will be tracked.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_pool_allocator_create(libd_memory_pool_allocator_s** out_allocator,
                                  size_t capacity,
                                  size_t datum_size,
                                  size_t freelist_capacity);

/**
 * @brief Destroys the arena.
 * @param p_arena Handle for the arena.
 * @return RESULT_OK on success, error code otherwise
 */
libd_memory_result_e
libd_memory_pool_allocator_destroy(libd_memory_pool_allocator_s* allocator);

/**
 * @brief Allocates memory in the arena.
 * @param p_arena Handle for the area.
 * @param pp_data Out parameter for the pointer to the allocation.
 * @return RESULT_OK on success, error code otherwise.
 */
libd_memory_result_e
libd_memory_pool_allocator_alloc(libd_memory_pool_allocator_s* allocator,
                                 void** out_param);

/**
 * @brief Frees the allocation for the given handle.
 * @param p_arena Handle for the allocator.
 * @param p_data Handle for the allocation to free.
 * @return RESULT_OK on success, error code otherwise.
 */
libd_memory_result_e
libd_memory_pool_allocator_free(libd_memory_pool_allocator_s* allocator,
                                void* p_data);

/**
 * @brief Resets the arena, freeing all allocations.
 * @param p_arena Handle for the arena.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_pool_allocator_reset(libd_memory_pool_allocator_s* allocator);

#endif  // LIBDANE_MEMORY_H
