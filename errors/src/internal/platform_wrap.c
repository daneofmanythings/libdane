#include "internal.h"
#include <libdane/platform/thread.h>
#include <stddef.h>

int
platform_thread_local_storage_create_once(void)
{

  libd_platform_thread_result_e result =
    libd_platform_thread_local_storage_create_once(NULL);
  if (result != RESULT_OK) {
    return result;
  }

  return RESULT_OK;
}

int
platform_thread_local_storage_get(void** pp_data, size_t size)
{
  libd_platform_thread_result_e result =
    libd_platform_thread_local_storage_get(NULL, pp_data, size);
  if (result != RESULT_OK) {
    return result;
  }
  return RESULT_OK;
}

int
platform_thread_local_storage_set(void* p_new_data, size_t size)
{
  void* p_data;
  libd_platform_thread_result_e result;

  result = platform_thread_local_storage_get(&p_data, size);
  if (result != RESULT_OK) {
    return result;
  }

  result = libd_platform_thread_local_storage_set(NULL, NULL, p_new_data, size);
  if (result != RESULT_OK) {
    return result;
  }

  return RESULT_OK;
}
