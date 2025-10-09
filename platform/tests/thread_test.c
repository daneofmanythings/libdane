#include "../include/libdane/platform/thread.h"

#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stdbool.h>

Test(thread_local_storage, should_pass) { cr_assert(true); }

Test(thread_local_storage, handle_create) {
  libd_platform_thread_local_storage_handle_s* handle;
  libd_platform_thread_result_e result =
    libd_platform_thread_local_storage_handle_create(&handle);

  cr_assert(eq(u8, result, RESULT_OK));

  libd_platform_thread_local_storage_handle_destroy(handle);
}

Test(thread_local_storage, storage_create) {
  libd_platform_thread_local_storage_handle_s* handle;
  libd_platform_thread_local_storage_handle_create(&handle);

  libd_platform_thread_result_e result =
    libd_platform_thread_local_storage_create_once(&handle);

  cr_assert(eq(u8, result, RESULT_OK));

  libd_platform_thread_local_storage_destroy(handle);
}
