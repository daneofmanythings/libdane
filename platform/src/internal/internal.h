/**
 * @file
 * @brief internal implementations and wrappers for the filesystem
 */

#ifndef LIBD_FILESYSTEM_INTERNAL_H
#define LIBD_FILESYSTEM_INTERNAL_H

#include <libdane/memory.h>

enum libd_allocator_result {
  ok,
  eof,
  oveflow,
  enomem,
};

struct libd_allocator_wrapper {
  libd_memory_linear_allocator_ot* a;
};

typedef void* (*alloc_f)(
  struct libd_allocator_wrapper*,
  size_t);
typedef void (*reset_f)(struct libd_allocator_wrapper*);

enum libd_allocator_result
libd_allocator_wrapper_create(
  struct libd_allocator_wrapper* allocator,
  size_t size,
  uint8_t alignment);

void*
libd_allocator_wrapper_alloc(
  struct libd_allocator_wrapper* allocator,
  size_t bytes);

void
libd_allocator_wrapper_reset(struct libd_allocator_wrapper* allocator);

void
libd_allocator_wrapper_destroy(struct libd_allocator_wrapper* allocator);

#endif  // LIBD_FILESYSTEM_INTERNAL_H
