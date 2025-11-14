#include "../../include/libd/memory.h"
#include "./internal/helpers.h"

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <unistd.h>

// field order matters for 16byte data alignment
struct linear_allocator {
  usize curr_data_size;
  usize head_index;
  u8 alignment;
  u8 header_size;
  u32 sys_page_size;
  usize data_reservation_size;
  u8 data[];
};

static inline usize
_total_reservation_size(struct linear_allocator* la)
{
  return libd_memory_align_up(
    la->header_size + la->data_reservation_size, la->sys_page_size);
}

struct linear_allocator_savepoint {
  u32 data_index;
};

enum libd_result
libd_linear_allocator_create(
  struct linear_allocator** out,
  u32 data_reservation_size,
  u32 starting_capacity,
  u8 alignment)
{
  if (out == NULL || starting_capacity == 0) {
    return libd_invalid_parameter;
  }

  if (!libd_memory_is_valid_alignment(alignment)) {
    return libd_invalid_alignment;
  }

  u8 header_size = sizeof(struct linear_allocator);

  s64 page_size = sysconf(_SC_PAGE_SIZE);
  if (page_size == -1) {
    return libd_err;
  }
  usize total_reservation_size =
    libd_memory_align_up(header_size + data_reservation_size, page_size);

  struct linear_allocator* la = mmap(
    NULL,
    total_reservation_size,
    PROT_NONE,
    MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE,
    -1,
    0);
  if (la == MAP_FAILED) {
    return libd_err;
  }

  usize curr_data_size = libd_memory_align_up(starting_capacity, page_size);
  usize curr_total_size =
    libd_memory_align_up(header_size + curr_data_size, page_size);

  if (mprotect(la, curr_total_size, PROT_READ | PROT_WRITE) != 0) {
    return libd_err;
  }

  la->curr_data_size        = curr_data_size;
  la->head_index            = 0;
  la->alignment             = alignment;
  la->data_reservation_size = data_reservation_size;
  la->sys_page_size         = page_size;
  la->header_size           = header_size;

  *out = la;

  return libd_ok;
}

enum libd_result
libd_linear_allocator_destroy(struct linear_allocator* la)
{
  if (la == NULL) {
    return libd_invalid_parameter;
  }

  if (munmap(la, _total_reservation_size(la)) != 0) {
    return libd_err;
  }

  return libd_ok;
}

enum libd_result
libd_linear_allocator_alloc(
  struct linear_allocator* la,
  void** out_ptr,
  u32 size)
{
  if (la == NULL || out_ptr == NULL) {
    return libd_invalid_parameter;
  }

  if (la->curr_data_size < la->head_index + size) {
    if (la->data_reservation_size < la->head_index + size) {
      return libd_no_memory;
    }
    // Round to next multiple of data_size * 2 to amortize future large
    // allocations
    usize new_data_size =
      libd_memory_align_up(la->head_index + size, la->curr_data_size * 2);

    // Ensure total allocation (header + data) is page-aligned for mprotect
    usize new_total_size =
      libd_memory_align_up(la->header_size + new_data_size, la->sys_page_size);

    new_total_size = MIN(new_total_size, _total_reservation_size(la));

    if (mprotect(la, new_total_size, PROT_READ | PROT_WRITE) != 0) {
      // TODO:
    }
    la->curr_data_size = new_total_size - la->header_size;
  }

  *out_ptr = &la->data[la->head_index];
  la->head_index += libd_memory_align_up(size, la->alignment);

  return libd_ok;
}

enum libd_result
libd_linear_allocator_set_savepoint(
  const struct linear_allocator* la,
  struct linear_allocator_savepoint* out_sp)
{
  if (la == NULL || out_sp == NULL) {
    return libd_invalid_parameter;
  }

  *out_sp = (struct linear_allocator_savepoint){
    .data_index = la->head_index,
  };

  return libd_ok;
}

enum libd_result
libd_linear_allocator_restore_savepoint(
  struct linear_allocator* la,
  const struct linear_allocator_savepoint* sp)
{
  if (la == NULL || sp == NULL) {
    return libd_invalid_parameter;
  }

  la->head_index = sp->data_index;

  return libd_ok;
}

enum libd_result
libd_linear_allocator_reset(struct linear_allocator* la)
{
  if (la == NULL) {
    return libd_invalid_parameter;
  }

  la->head_index = 0;

  return libd_ok;
}

enum libd_result
libd_linear_allocator_bytes_free(
  struct linear_allocator* la,
  usize* out_size_bytes)
{
  if (la == NULL || out_size_bytes == NULL) {
    return libd_invalid_parameter;
  }

  *out_size_bytes = la->curr_data_size - la->head_index;

  return libd_ok;
}
