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
  LIBD_MEM_OK,                     /**< Operation successful */
  LIBD_MEM_NO_MEMORY,              /**< No memory for the operation */
  LIBD_MEM_NOT_IMPLEMENTED,        /**< Functionality not yet implemented */
  LIBD_MEM_INVALID_ALIGNMENT,      /**< An invalid alignment value was given */
  LIBD_MEM_INVALID_NULL_PARAMETER, /**< A NULL pointer parameter was detected */
  LIBD_MEM_INVALID_POINTER, /**< An invalid pointer parameter was detected */
  LIBD_MEM_INVALID_ZERO_PARAMETER, /**< An invalid zero value as a parameter */
  LIBD_MEM_INVALID_FREE,   /**< Tried to free memory from an arena that has no
                         allocations */
  LIBD_MEM_RESULT_E_COUNT, /**< Count of result states */
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
 * @brief Opaque handle for the slab allocator.
 */
typedef struct libd_memory_slab_allocator_s libd_memory_slab_allocator_s;

/**
 * @brief Allocator callback signature
 */
typedef void* (*libd_memory_allocator_callback_f)(void*, size_t);

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
libd_memory_align_value(size_t offset, uint8_t alignment)
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
// Slab Allocator API
//==============================================================================

/**
 * @brief Handles a fixed size/alignment allocations and supports freeing of
 * individual allocations. Simple implementation with no regard for page sizing
 * optimizations.
 * @warning The free list is handled as an embedded ring buffer with capacity
 * equal to the capacity of the allocator. The footprint may be large. There is
 * no algorithm to address possible fragmentation of the slab.
 * @param out_allocator Out parameter for the allocator.
 * @param max_allocations Maximum number of allocations.
 * @param bytes_per_alloc Size, in bytes, per allocation.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_slab_allocator_create(libd_memory_slab_allocator_s** out_allocator,
                                  size_t max_allocations,
                                  size_t bytes_per_alloc,
                                  uint8_t alignment);

/**
 * @brief Destroys the allocator.
 * @param p_allocator Handle for the allocator.
 * @return RESULT_OK on success, error code otherwise
 */
libd_memory_result_e
libd_memory_slab_allocator_destroy(libd_memory_slab_allocator_s* p_allocator);

/**
 * @brief Allocates memory in a free block.
 * @param p_allocator Handle for the allocator.
 * @param out_pointer Out parameter for the pointer to the allocation.
 * @return RESULT_OK on success, error code otherwise.
 */
libd_memory_result_e
libd_memory_slab_allocator_alloc(libd_memory_slab_allocator_s* p_allocator,
                                 void** out_pointer);

/**
 * @brief Frees the allocation for the given handle.
 * @param p_allocator Handle for the allocator.
 * @param p_to_free Handle for the allocation to free.
 * @return RESULT_OK on success, error code otherwise.
 */
libd_memory_result_e
libd_memory_slab_allocator_free(libd_memory_slab_allocator_s* p_allocator,
                                void* p_to_free);

/**
 * @brief Resets the slab, freeing all allocations.
 * @param p_allocator Handle for the allocator.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_memory_result_e
libd_memory_slab_allocator_reset(libd_memory_slab_allocator_s* p_allocator);

#endif  // LIBDANE_MEMORY_H
