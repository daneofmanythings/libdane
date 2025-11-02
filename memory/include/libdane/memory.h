/**
 * @file memory.h
 * @brief Platform-specific thread local storage interface
 */

#ifndef LIBDANE_MEMORY_H
#define LIBDANE_MEMORY_H

#include "../../../include/types.h"
#include "internal/align_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//==============================================================================
// Type Definitions
//==============================================================================

/**
 * @brief Opaque handle for the linear allocator.
 */
typedef struct linear_allocator libd_linear_allocator_h;

/**
 * @brief Opaque handle to a linear allocator checkpoint;
 */
typedef struct linear_allocator_savepoint libd_linear_allocator_savepoint_h;

/**
 * @brief Opaque handle for the slab allocator.
 */
typedef struct pool_allocator libd_pool_allocator_h;

//==============================================================================
// Inline Helper Functions
//==============================================================================

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

/**
 * @brief Validates the given alignment. Must be a non-zero power of 2 that
 * doesn't exceed the max alignment.
 * @param alignment Value to verify.
 * @return LIBD_MEM_OK on success, non-zero otherwise.
 */
static inline enum libd_result
libd_memory_is_valid_alignment(uint8_t alignment)
{
  if (alignment == 0) {
    return libd_invalid_alignment;
  }

  if (!libd_memory_is_power_of_two(alignment) || alignment > LIBD_MAX_ALIGN) {
    return libd_invalid_alignment;
  }

  return libd_ok;
}

/**
 * @brief Rounds the value to the nearest given alignment value.
 * @warning alignment must be non-zero.
 * @param value The value to round from.
 * @param alignment The power of 2 to round to. This parameter MUST be a power
 * of 2.
 */
static inline uintptr_t
libd_memory_align_value(
  uintptr_t value,
  uint8_t alignment)
{
  if (libd_memory_is_valid_alignment(alignment) != libd_ok) {
    return libd_invalid_alignment;
  }

  return ((value + alignment - 1) & ~(uintptr_t)(alignment - 1));
}

//==============================================================================
// Linear Allocator API
//==============================================================================

/**
 * @brief Creates a linear allocator.
 * @param out_allocator Out parameter for the allocator.
 * @param capacity_bytes Capacity for the arena in bytes.
 * @param alignment Alignment value for the allocator. Must be a power of 2.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_create(
  libd_linear_allocator_h** out_allocator,
  size_t capacity_bytes,
  uint8_t alignment);

/**
 * @brief Destroys the allocator.
 * @param p_allocator Handle for the arena.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_destroy(libd_linear_allocator_h* p_allocator);

/**
 * @brief Allocates memory in the arena.
 * @param p_allocator Handle to the arena.
 * @param out_pointer Out parameter for the pointer to the allocation.
 * @param size_bytes Size in bytes for the allocation.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_alloc(
  libd_linear_allocator_h* p_allocator,
  void** out_pointer,
  size_t size_bytes);

/**
 * @brief Sets a savepoint which can be restored to.
 * @param p_allocator Handle to the allocator.
 * @param out_savepoint Out parameter for the savepoint.
 * @returns libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_set_savepoint(
  const libd_linear_allocator_h* p_allocator,
  libd_linear_allocator_savepoint_h* out_savepoint);

/**
 * @brief Restores the allocator state to when the savepoint was created by
 * freeing all allocations performed after the savepoint.
 * @warning Restoring the allocator to a savepoint invalidates all savepoints
 * created after it. Attempting to restore to a savepoint that has been
 * invalidated is undefined behavior.
 * @param p_allocator Handle to the allocator.
 * @param p_savepoint Handle to the savepoint.
 * @returns libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_restore_savepoint(
  libd_linear_allocator_h* p_allocator,
  const libd_linear_allocator_savepoint_h* p_savepoint);

/**
 * @brief Resets the allocator to the default state by freeing all allocations.
 * @param p_allocator Handle to the allocator.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_reset(libd_linear_allocator_h* p_allocator);

/**
 * @brief Introspects the remaining number of bytes for allocation.
 * @param p_allocator Handle to the allocator.
 * @param out_size_bytes Out parameter for the remaining number of bytes.
 * @return libd_mem_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_bytes_free(
  libd_linear_allocator_h* p_allocator,
  size_t* out_size_bytes);

//==============================================================================
// Pool Allocator API
//==============================================================================

/**
 * @return libd_ok on success, non-zero otherwise
 */
enum libd_result
libd_pool_allocator_create(
  libd_pool_allocator_h** out_allocator,
  uint32_t max_allocations,
  uint32_t bytes_per_alloc,
  uint8_t alignment);

/**
 * @return libd_ok on success, non-zero otherwise
 */
enum libd_result
libd_pool_allocator_destroy(libd_pool_allocator_h* p_allocator);

/**
 * @brief Allocates memory in a free block.
 * @param p_allocator Handle for the allocator.
 * @param out_pointer Out parameter for the pointer to the allocation.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_pool_allocator_alloc(
  libd_pool_allocator_h* p_allocator,
  void** out_pointer);

/**
 * @brief Frees the allocation for the given handle.
 * @param p_allocator Handle for the allocator.
 * @param p_to_free Handle for the allocation to free.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_pool_allocator_free(
  libd_pool_allocator_h* p_allocator,
  void* p_to_free);

/**
 * @brief Resets the pool, freeing all allocations.
 * @param p_allocator Handle for the allocator.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_pool_allocator_reset(libd_pool_allocator_h* p_allocator);

#endif  // LIBDANE_MEMORY_H
