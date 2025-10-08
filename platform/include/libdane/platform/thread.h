#ifndef LIBD_PLATFORM_THREAD_H
#define LIBD_PLATFORM_THREAD_H

#include <stddef.h>
typedef enum {
  RESULT_OK,
  LIBD_PLATFORM_RESULT_E_COUNT,
} libd_platform_thread_result_e;

typedef int (*libd_platform_thread_local_storage_data_setter_f)(void*, void*);

typedef struct libd_platform_thread_local_storage_handle_s libd_platform_thread_local_storage_handle_s;

libd_platform_thread_result_e libd_platform_thread_local_storage_handle_create(libd_platform_thread_local_storage_handle_s** p_handle);
void libd_platform_thread_local_storage_handle_destroy(libd_platform_thread_local_storage_handle_s* handle);

libd_platform_thread_result_e libd_platform_thread_local_storage_create_once(libd_platform_thread_local_storage_handle_s** pp_handle);
void libd_platform_thread_local_storage_destroy(libd_platform_thread_local_storage_handle_s* p_handle);
libd_platform_thread_result_e libd_platform_thread_local_storage_get(libd_platform_thread_local_storage_handle_s* p_handle, void** data, size_t size);
libd_platform_thread_result_e libd_platform_thread_local_storage_set(libd_platform_thread_local_storage_handle_s* p_handle,
                                                                     libd_platform_thread_local_storage_data_setter_f setter, void* new_data,
                                                                     size_t size);

#endif // !LIBD_PLATFORM_THREAD_H
