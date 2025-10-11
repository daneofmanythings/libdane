#ifndef LIBD_ERRORS_INTERNAL_H
#define LIBD_ERRORS_INTERNAL_H

#include <stddef.h>

int
platform_thread_local_static_get(void** pp_data, size_t size);

int
platform_thread_local_static_set(void* p_new_data, size_t size);

#endif  // !LIBD_ERRORS_INTERNAL_H
