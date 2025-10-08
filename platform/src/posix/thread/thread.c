#include "../../../include/libdane/platform/thread.h"

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pthread_key_t err_key;
static pthread_once_t err_once = PTHREAD_ONCE_INIT;

static void _thread_local_storage_free(void* data);
static void _thread_local_storage_init(void);
static int _thread_local_storage_init_once(void);

struct libd_platform_thread_local_storage_handle_s {
  //
};

// No-op for this platfrom
libd_platform_thread_result_e libd_platform_thread_local_storage_handle_create(libd_platform_thread_local_storage_handle_s** pp_handle) {
  return RESULT_OK;
}

// No-op for this platform
void libd_platform_thread_local_storage_handle_destroy(libd_platform_thread_local_storage_handle_s* handle) {
  //
}

libd_platform_thread_result_e libd_platform_thread_local_storage_create_once(libd_platform_thread_local_storage_handle_s** pp_handle) {
  int err = _thread_local_storage_init_once();
  if (err != 0) {
    return err;
  }
  return RESULT_OK;
}

void libd_platform_thread_local_storage_destroy(libd_platform_thread_local_storage_handle_s* p_handle) {
  //
}

libd_platform_thread_result_e libd_platform_thread_local_storage_set(libd_platform_thread_local_storage_handle_s* p_handle,
                                                                     libd_platform_thread_local_storage_data_setter_f setter_f, void* new_data,
                                                                     size_t size) {
  void* thread_local_data;
  int err = libd_platform_thread_local_storage_get(p_handle, &thread_local_data, size);
  if (err != 0) {
    return err;
  }

  if (setter_f != NULL) {
    setter_f(thread_local_data, new_data);
  } else {
    memcpy(thread_local_data, new_data, size);
  }

  return 0;
}

libd_platform_thread_result_e libd_platform_thread_local_storage_get(libd_platform_thread_local_storage_handle_s* p_handle, void** pp_data,
                                                                     size_t data_size) {
  libd_platform_thread_result_e result = libd_platform_thread_local_storage_create_once(NULL);
  if (result != RESULT_OK) {
    return result;
  }
  void* p_data = pthread_getspecific(err_key);

  if (p_data == NULL) {
    p_data = malloc(data_size);
    if (p_data == NULL) {
      // TODO: make errors;
      return 1;
    }
    pthread_setspecific(err_key, p_data);
  }

  *pp_data = p_data;

  return 0;
}

static int _thread_local_storage_init_once(void) {
  // TODO: somehow get the execution of _thread_local_storage_init to pass on a possible error.
  return pthread_once(&err_once, _thread_local_storage_init);
}

static void _thread_local_storage_init(void) {
  if (pthread_key_create(&err_key, _thread_local_storage_free) != 0) {

    // NOTE: I dont like this error handling :(
    fprintf(stderr, "ERROR: could not create thread local storage key\n");
    exit(1);
    // ----
  }
}
static void _thread_local_storage_free(void* data) {
  if (data != NULL) {
    free(data);
  }
}
