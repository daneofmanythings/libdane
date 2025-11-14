/**
 * @file memory.h
 * @brief Platform-specific thread local storage interface
 */

#ifndef LIBDANE_MEMORY_H
#define LIBDANE_MEMORY_H

#include "common.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

//==============================================================================
// Type Definitions
//==============================================================================

/**
 * @brief Opaque handle for the linear allocator.
 * @note The allocator's data region is aligned to 16 bytes.
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
// Linear Allocator API
//==============================================================================

/**
 * @brief Creates a linear allocator.
 * @param out Out parameter for the allocator.
 * @param reservation_size_bytes The amount of virtual address space to reserve
 * for the allocator.
 * @param starting_capacity_bytes Amount of bytes to initialize. Rounds to page
 * boundaries.
 * @param alignment Alignment value for the allocator. Must be a non-zer power
 * of 2.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_create(
  struct linear_allocator** out,
  u32 reservation_size_bytes,
  u32 starting_capacity_bytes,
  u8 alignment);

/**
 * @brief Destroys the allocator.
 * @param la Handle for the arena.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_destroy(libd_linear_allocator_h* la);

/**
 * @brief Allocates memory in the arena, returning a void pointer through the
 * out parameter.
 * @param p_allocator Handle to the arena.
 * @param out_pointer Out parameter for the pointer to the allocation.
 * @param size_bytes Size in bytes for the allocation.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_alloc(
  libd_linear_allocator_h* la,
  void** out,
  u32 size_bytes);

/**
 * @brief Sets a savepoint which can be restored to.
 * @param la Handle to the allocator.
 * @param out Out parameter for the savepoint.
 * @returns libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_set_savepoint(
  const libd_linear_allocator_h* la,
  libd_linear_allocator_savepoint_h* out);

/**
 * @brief Restores the allocator state to when the savepoint was created by
 * freeing all allocations performed after the savepoint.
 * @warning Restoring the allocator to a savepoint invalidates all savepoints
 * created after it. Attempting to restore to a savepoint that has been
 * invalidated is undefined behavior.
 * @param la Handle to the allocator.
 * @param sp Handle to the savepoint.
 * @returns libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_restore_savepoint(
  libd_linear_allocator_h* la,
  const libd_linear_allocator_savepoint_h* sp);

/**
 * @brief Resets the allocator to the default state by freeing all allocations.
 * @param la Handle to the allocator.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_reset(libd_linear_allocator_h* la);

/**
 * @brief Introspects the number of availible bytes for allocation.
 * @param la Handle to the allocator.
 * @param out Out parameter for the remaining number of bytes.
 * @return libd_mem_ok on success, non-zero otherwise.
 */
enum libd_result
libd_linear_allocator_bytes_free(
  libd_linear_allocator_h* la,
  size_t* out);

//==============================================================================
// Pool Allocator API
//==============================================================================

/**
 * @return libd_ok on success, non-zero otherwise
 */
enum libd_result
libd_pool_allocator_create(
  libd_pool_allocator_h** pa,
  u32 max_allocations,
  u32 bytes_per_alloc,
  u8 alignment);

/**
 * @return libd_ok on success, non-zero otherwise
 */
enum libd_result
libd_pool_allocator_destroy(libd_pool_allocator_h* pa);

/**
 * @brief Allocates memory in a free block.
 * @param pa Handle for the allocator.
 * @param out Out parameter for the pointer to the allocation.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_pool_allocator_alloc(
  libd_pool_allocator_h* pa,
  void** out);

/**
 * @brief Frees the allocation for the given handle.
 * @param pa Handle for the allocator.
 * @param ptr Handle for the allocation to free.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_pool_allocator_free(
  libd_pool_allocator_h* pa,
  void* ptr);

/**
 * @brief Resets the pool, freeing all allocations.
 * @param pa Handle for the allocator.
 * @return libd_ok on success, non-zero otherwise.
 */
enum libd_result
libd_pool_allocator_reset(libd_pool_allocator_h* pa);

#endif  // LIBDANE_MEMORY_H
