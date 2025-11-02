#ifndef LIBD_ERRORS_INTERNAL_H
#define LIBD_ERRORS_INTERNAL_H

#include "../../../include/types.h"

#include <stddef.h>

enum libd_result
platform_thread_local_static_get(
  void** pp_data,
  size_t size);

enum libd_result
platform_thread_local_static_set(
  void* p_new_data,
  size_t size);

#endif  // !LIBD_ERRORS_INTERNAL_H
