#include "../../../include/libdane/platform/thread.h"

#include <libdane/platform/thread.h>
#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


struct libd_platform_thread_local_storage_handle_s {
  pthread_key_t key;
  size_t data_size;
};

// Convenience typedefs
typedef struct libd_platform_thread_local_storage_handle_s handle_s;
typedef libd_platform_thread_local_storage_destructor_f destructor_f;
typedef libd_platform_thread_local_storage_data_setter_f data_setter_f;
typedef libd_platform_thread_result_e result_e;

// singleton storage slot variables
static handle_s* g_static_handle = NULL;
static pthread_once_t g_static_once = PTHREAD_ONCE_INIT;
static destructor_f g_static_destructor = NULL;
static size_t g_static_size = 0;

result_e
libd_platform_thread_local_storage_create(handle_s** p_handle,
                                          destructor_f destructor,
                                          size_t size)
{
  handle_s* handle = malloc(sizeof(handle_s));
  if (handle == NULL) {
    // TODO:
    return LIBD_PF_THREAD_NO_MEMORY;
  }

  handle->data_size = size;

  if (pthread_key_create(&handle->key, destructor) != 0) {
    free(handle);
    return LIBD_PF_THREAD_NO_MEMORY;
  }

  *p_handle = handle;

  return LIBD_PF_THREAD_OK;
}

result_e
libd_platform_thread_local_storage_destroy(handle_s* handle)
{
  if (handle == NULL) {
    return LIBD_PF_THREAD_NULL_PARAMETER;
  }

  pthread_key_delete(handle->key);
  free(handle);

  return LIBD_PF_THREAD_OK;
}

result_e
libd_platform_thread_local_storage_get(handle_s* handle, void** pp_data)
{
  if (handle == NULL) {
    return LIBD_PF_THREAD_NULL_PARAMETER;
  }

  void* p_data = pthread_getspecific(handle->key);

  if (p_data == NULL) {
    p_data = malloc(handle->data_size);
    if (p_data == NULL) {
      return LIBD_PF_THREAD_NO_MEMORY;
    }
    if (pthread_setspecific(handle->key, p_data) != 0) {
      free(p_data);
      // TODO:
      return 1;
    }
  }

  *pp_data = p_data;

  return LIBD_PF_THREAD_OK;
}

result_e
libd_platform_thread_local_storage_set(handle_s* handle,
                                       data_setter_f setter_f,
                                       void* new_data)
{
  if (handle == NULL || new_data == NULL) {
    return LIBD_PF_THREAD_NULL_PARAMETER;
  }

  void* p_data;
  result_e result = libd_platform_thread_local_storage_get(handle, &p_data);
  if (result != LIBD_PF_THREAD_OK) {
    return result;
  }

  if (setter_f != NULL) {
    setter_f(p_data, new_data);
    return LIBD_PF_THREAD_OK;
  }

  memcpy(p_data, new_data, handle->data_size);

  return LIBD_PF_THREAD_OK;
}

static void
_init_static_thread_local_storage(void);

result_e
libd_platform_thread_local_static_init(destructor_f destructor, size_t size)
{
  if (g_static_handle != NULL) {
    return LIBD_PF_THREAD_OK;
  }

  g_static_destructor = destructor;
  g_static_size = size;
  pthread_once(&g_static_once, _init_static_thread_local_storage);

  if (g_static_handle == NULL) {
    return LIBD_PF_THREAD_INIT_FAILED;
  }

  return LIBD_PF_THREAD_OK;
}

result_e
libd_platform_thread_local_static_get(void** pp_data)
{
  if (g_static_handle == NULL) {
    return LIBD_PF_THREAD_NOT_INITIALIZED;
  }
  return libd_platform_thread_local_storage_get(g_static_handle, pp_data);
}

result_e
libd_platform_thread_local_static_set(data_setter_f setter, void* data)
{
  if (g_static_handle == NULL) {
    return LIBD_PF_THREAD_NOT_INITIALIZED;
  }
  return libd_platform_thread_local_storage_set(g_static_handle, setter, data);
}

result_e
libd_platform_thread_local_static_cleanup(void)
{
  if (g_static_handle == NULL) {
    return LIBD_PF_THREAD_NOT_INITIALIZED;
  }
  return libd_platform_thread_local_storage_destroy(g_static_handle);
}

static void
_init_static_thread_local_storage(void)
{
  libd_platform_thread_result_e result =
    libd_platform_thread_local_storage_create(
      &g_static_handle, g_static_destructor, g_static_size);

  if (result != LIBD_PF_THREAD_OK) {
    fprintf(stderr, "Failed to create thread-local storage for the singleton "
                    "storage element.\n");
    exit(result);
  }
}
