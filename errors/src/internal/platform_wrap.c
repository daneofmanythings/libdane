#include "internal.h"

#include <libdane/platform/thread.h>
#include <stddef.h>
#include <stdlib.h>

int
platform_thread_local_static_get(void** pp_data, size_t size)
{
  if (libd_platform_thread_local_static_init(NULL, size) != 0) {
    return ERR_INIT_FAILED;
  }
  return libd_platform_thread_local_static_get(pp_data);
}

int
platform_thread_local_static_set(void* p_new_data, size_t size)
{
  if (libd_platform_thread_local_static_init(NULL, size) != 0) {
    return ERR_INIT_FAILED;
  }
  return libd_platform_thread_local_static_set(NULL, p_new_data);
}
