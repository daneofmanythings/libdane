#include "../../../platform/include/libdane/platform/thread.h"
#include "internal.h"

#include <stddef.h>
#include <stdlib.h>

enum libd_result
platform_thread_local_static_get(
  void** pp_data,
  size_t size)
{
  return libd_platform_thread_local_static_get(pp_data);
}

enum libd_result
platform_thread_local_static_set(
  void* p_new_data,
  size_t size)
{
  return libd_platform_thread_local_static_set(NULL, p_new_data);
}
