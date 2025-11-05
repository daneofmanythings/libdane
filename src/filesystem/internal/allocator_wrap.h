/**
 * @file
 * @brief internal implementations and wrappers for the filesystem
 */

#ifndef LIBD_FILESYSTEM_ALLOCATOR_WRAP_H
#define LIBD_FILESYSTEM_ALLOCATOR_WRAP_H

#include "../../../include/libd/memory.h"

struct filepath_allocator_wrapper {
  libd_linear_allocator_h* a;
};

typedef void* (*alloc_f)(
  struct filepath_allocator_wrapper*,
  size_t);
typedef void (*reset_f)(struct filepath_allocator_wrapper*);

enum libd_result
libd_allocator_wrapper_create(
  struct filepath_allocator_wrapper* allocator,
  size_t size,
  uint8_t alignment);

void*
libd_allocator_wrapper_alloc(
  struct filepath_allocator_wrapper* allocator,
  size_t bytes);

void
libd_allocator_wrapper_reset(struct filepath_allocator_wrapper* allocator);

void
libd_allocator_wrapper_destroy(struct filepath_allocator_wrapper* allocator);

#endif  // LIBD_FILESYSTEM_ALLOCATOR_WRAP_H
