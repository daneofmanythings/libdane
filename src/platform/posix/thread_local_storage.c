#include "../../../include/libd/platform/threads.h"

#include <pthread.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct thread_local_storage_handle {
  pthread_key_t key;
  size_t data_size;
};

// singleton storage slot variables
static struct thread_local_storage_handle* g_static_handle = NULL;
static pthread_once_t g_static_once                        = PTHREAD_ONCE_INIT;
static size_t g_static_size                                = 0;
static libd_platform_thread_local_storage_destructor_f g_static_destructor =
  NULL;

enum libd_result
libd_platform_thread_local_storage_create(
  struct thread_local_storage_handle** p_handle,
  libd_platform_thread_local_storage_destructor_f destructor,
  size_t size)
{
  struct thread_local_storage_handle* handle =
    malloc(sizeof(struct thread_local_storage_handle));
  if (handle == NULL) {
    // TODO:
    return libd_no_memory;
  }

  handle->data_size = size;

  if (pthread_key_create(&handle->key, destructor) != 0) {
    free(handle);
    return libd_no_memory;
  }

  *p_handle = handle;

  return libd_ok;
}

enum libd_result
libd_platform_thread_local_storage_destroy(
  struct thread_local_storage_handle* handle)
{
  if (handle == NULL) {
    return libd_invalid_parameter;
  }

  pthread_key_delete(handle->key);
  free(handle);

  return libd_ok;
}

enum libd_result
libd_platform_thread_local_storage_get(
  struct thread_local_storage_handle* handle,
  void** pp_data)
{
  if (handle == NULL) {
    return libd_invalid_parameter;
  }

  void* p_data = pthread_getspecific(handle->key);

  if (p_data == NULL) {
    p_data = malloc(handle->data_size);
    if (p_data == NULL) {
      return libd_no_memory;
    }
    if (pthread_setspecific(handle->key, p_data) != 0) {
      free(p_data);
      // TODO:
      return 1;
    }
  }

  *pp_data = p_data;

  return libd_ok;
}

enum libd_result
libd_platform_thread_local_storage_set(
  struct thread_local_storage_handle* handle,
  libd_platform_thread_local_storage_data_setter_f setter_f,
  void* new_data)
{
  if (handle == NULL || new_data == NULL) {
    return libd_invalid_parameter;
  }

  void* p_data;
  enum libd_result result =
    libd_platform_thread_local_storage_get(handle, &p_data);
  if (result != libd_ok) {
    return result;
  }

  if (setter_f != NULL) {
    setter_f(p_data, new_data);
    return libd_ok;
  }

  memcpy(p_data, new_data, handle->data_size);

  return libd_ok;
}

static void
_init_static_thread_local_storage(void);

enum libd_result
libd_platform_thread_local_static_init(
  libd_platform_thread_local_storage_destructor_f destructor,
  size_t size)
{
  if (g_static_handle != NULL) {
    return libd_ok;
  }

  g_static_destructor = destructor;
  g_static_size       = size;
  pthread_once(&g_static_once, _init_static_thread_local_storage);

  if (g_static_handle == NULL) {
    return libd_thread_init_failed;
  }

  return libd_ok;
}

enum libd_result
libd_platform_thread_local_static_get(void** pp_data)
{
  if (g_static_handle == NULL) {
    return libd_not_initialized;
  }
  return libd_platform_thread_local_storage_get(g_static_handle, pp_data);
}

enum libd_result
libd_platform_thread_local_static_set(
  libd_platform_thread_local_storage_data_setter_f setter,
  void* data)
{
  if (g_static_handle == NULL) {
    return libd_not_initialized;
  }
  return libd_platform_thread_local_storage_set(g_static_handle, setter, data);
}

enum libd_result
libd_platform_thread_local_static_cleanup(void)
{
  if (g_static_handle == NULL) {
    return libd_not_initialized;
  }
  return libd_platform_thread_local_storage_destroy(g_static_handle);
}

static void
_init_static_thread_local_storage(void)
{
  enum libd_result result = libd_platform_thread_local_storage_create(
    &g_static_handle, g_static_destructor, g_static_size);

  if (result != libd_ok) {
    fprintf(
      stderr,
      "Failed to create thread-local storage for the singleton "
      "storage element.\n");
    exit(result);
  }
}
