#include "../../include/libd/memory.h"
#include "../../include/libd/utils/align_compat.h"
#include "./internal/helpers.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

struct pool_allocator {
  u32 max_allocations;
  u32 bytes_per_alloc;
  u32 head_index;
  u32 tail_index;
  u8 data[];
};

static inline u32
_byte_index(
  struct pool_allocator* pa,
  u32 i);

static inline u32
_head_byte_index(struct pool_allocator* pa);

static inline u32
_tail_byte_index(struct pool_allocator* pa);

static inline u32
_terminal_index(struct pool_allocator* pa);

static inline u8*
_ptr_to_index(
  struct pool_allocator* pa,
  u32 i);

enum libd_result
_initialize_free_list(struct pool_allocator* pa);

// NOTE: not used in the code, but here for size and alignment calculations,
// with the possibility of future extension.
typedef struct {
  u32 next_free_slot_index;
} free_node;
static inline usize
_aligned_sizeof_free_node()
{
  return libd_memory_align_up(sizeof(free_node), LIBD_ALIGNOF(free_node));
}

enum libd_result
libd_pool_allocator_create(
  struct pool_allocator** out_pa,
  u32 max_allocations,
  u32 bytes_per_alloc,
  u8 alignment)
{
  if (out_pa == NULL || max_allocations == 0 || bytes_per_alloc == 0) {
    return libd_invalid_parameter;
  }
  if (!libd_memory_is_valid_alignment(alignment)) {
    return libd_invalid_alignment;
  }
  // checking that there is room for the terminal index
  if (max_allocations == U32_MAX) {
    return libd_no_memory;
  }

  u32 aligned_bytes_per_alloc =
    libd_memory_align_up(bytes_per_alloc, alignment);

  // Ensure there is enough space for the free list nodes.
  if (aligned_bytes_per_alloc < _aligned_sizeof_free_node()) {
    aligned_bytes_per_alloc = _aligned_sizeof_free_node();
  }

  usize data_size  = (max_allocations + 1) * aligned_bytes_per_alloc;
  usize total_size = sizeof(struct pool_allocator) + data_size + alignment - 1;
  struct pool_allocator* pa = malloc(total_size);
  if (pa == NULL) {
    return libd_no_memory;
  }

  pa->max_allocations = max_allocations;
  pa->bytes_per_alloc = aligned_bytes_per_alloc;

  if (_initialize_free_list(pa) != libd_ok) {
    free(pa);
    return libd_free_list_failure;
  }

  *out_pa = pa;

  return libd_ok;
}

enum libd_result
libd_pool_allocator_destroy(struct pool_allocator* pa)
{
  if (pa == NULL) {
    return libd_invalid_parameter;
  }

  free(pa);

  return libd_ok;
}

enum libd_result
libd_pool_allocator_alloc(
  struct pool_allocator* pa,
  void** out_pointer)
{
  if (pa == NULL || out_pointer == NULL) {
    return libd_invalid_parameter;
  }

  if (pa->head_index == _terminal_index(pa)) {
    return libd_no_memory;
  }

  // setting the out pointer to the allocated region.
  *out_pointer = _ptr_to_index(pa, pa->head_index);

  // copying the next index into head.
  memcpy(&pa->head_index, *out_pointer, sizeof(free_node));

  // If the last slot was just allocated, move the tail as well.
  if (pa->head_index == _terminal_index(pa)) {
    pa->tail_index = _terminal_index(pa);
  }

  return libd_ok;
}

enum libd_result
libd_pool_allocator_free(
  struct pool_allocator* pa,
  void* p_to_free)
{
  if (pa == NULL || p_to_free == NULL) {
    return libd_invalid_parameter;
  }

  ptrdiff_t byte_offset = (u8*)p_to_free - pa->data;
  if (byte_offset < 0 || byte_offset % pa->bytes_per_alloc != 0) {
    return libd_invalid_pointer;  // below bounds or not block aligned
  }

  u32 free_index = byte_offset / pa->bytes_per_alloc;
  if (free_index >= pa->max_allocations) {
    return libd_invalid_pointer;  // above bounds
  }

  if (free_index < pa->head_index) {
    // linking the new head to the previous head index.
    memcpy(_ptr_to_index(pa, free_index), &pa->head_index, sizeof(free_node));

    // updating the head to point at the new index.
    pa->head_index = free_index;
  } else {
    // update the old tail with the index of the newly freed slot.
    memcpy(_ptr_to_index(pa, pa->tail_index), &free_index, sizeof(free_node));
    // update the tail index
    pa->tail_index = free_index;

    // point the new tail to the terminal index
    u32 temp = _terminal_index(pa);
    memcpy(_ptr_to_index(pa, pa->tail_index), &temp, sizeof(free_node));
  }

  // if we just free'd out of a full allocator, move the tail as well.
  if (pa->tail_index == _terminal_index(pa)) {
    pa->tail_index = pa->head_index;
  }

  return libd_ok;
}

enum libd_result
libd_pool_allocator_reset(struct pool_allocator* pa)
{
  return _initialize_free_list(pa);
}

enum libd_result
_initialize_free_list(struct pool_allocator* pa)
{
  pa->head_index = 0;
  pa->tail_index = _terminal_index(pa) - 1;
  u32 next_index;
  for (u32 i = 0; i < pa->tail_index; i += 1) {
    next_index = i + 1;
    memcpy(_ptr_to_index(pa, i), &next_index, sizeof(free_node));
  }

  next_index = _terminal_index(pa);
  memcpy(_ptr_to_index(pa, pa->tail_index), &next_index, sizeof(free_node));

  return libd_ok;
}

static inline u32
_byte_index(
  struct pool_allocator* pa,
  u32 i)
{
  return i * pa->bytes_per_alloc;
}

static inline u32
_head_byte_index(struct pool_allocator* pa)
{
  return pa->head_index * pa->bytes_per_alloc;
}

static inline u32
_tail_byte_index(struct pool_allocator* pa)
{
  return pa->tail_index * pa->bytes_per_alloc;
}

static inline u32
_terminal_index(struct pool_allocator* pa)
{
  return pa->max_allocations;
}

static inline u8*
_ptr_to_index(
  struct pool_allocator* pa,
  u32 i)
{
  return &pa->data[_byte_index(pa, i)];
}
