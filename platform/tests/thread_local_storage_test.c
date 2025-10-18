#include "../include/libdane/platform/thread.h"

#include <criterion/alloc.h>
#include <criterion/criterion.h>
#include <criterion/internal/new_asserts.h>
#include <criterion/new/assert.h>
#include <stdbool.h>

typedef struct {
  int member;
} test_s;

typedef struct {
  test_s* nested;
} test_nest_s;

Test(handle, create)
{
  libd_platform_thread_local_storage_handle_s* handle;
  libd_platform_thread_result_e result;
  result =
    libd_platform_thread_local_storage_create(&handle, free, sizeof(test_s));
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));
}

Test(handle, destroy)
{
  libd_platform_thread_local_storage_handle_s* handle;
  libd_platform_thread_result_e result;
  result =
    libd_platform_thread_local_storage_create(&handle, free, sizeof(test_s));
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));

  result = libd_platform_thread_local_storage_destroy(handle);
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));
}

Test(handle, set_get_flat)
{
  int test_val = 10;
  libd_platform_thread_local_storage_handle_s* handle;
  libd_platform_thread_result_e result;
  result =
    libd_platform_thread_local_storage_create(&handle, free, sizeof(test_s));
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));

  test_s data_source = {.member = test_val};
  result = libd_platform_thread_local_storage_set(handle, NULL, &data_source);
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));

  test_s data_dest = {.member = 0};
  test_s* p_data_dest = &data_dest;
  result = libd_platform_thread_local_storage_get(handle, (void**)&p_data_dest);
  cr_assert(eq(u8, p_data_dest->member, test_val));

  result = libd_platform_thread_local_storage_destroy(handle);
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));
}

static int
_test_nest_setter_f(void* dest, void* src)
{
  test_nest_s* t_dest = (void*)dest;
  test_nest_s* t_src = (void*)src;

  t_dest->nested = t_src->nested;

  return LIBD_PF_THREAD_OK;
}
static void
_test_nest_destructor_f(void* dat)
{
  test_nest_s* t = (test_nest_s*)dat;
  if (t->nested != NULL) {
    free(t->nested);
  }
}
Test(handle, set_get_nested)
{
  int test_val = 10;
  libd_platform_thread_local_storage_handle_s* handle;
  libd_platform_thread_result_e result;
  result = libd_platform_thread_local_storage_create(
    &handle, _test_nest_destructor_f, sizeof(test_s));
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));

  test_s* payload = malloc(sizeof(test_s));
  cr_assert(ne(ptr, payload, NULL));
  payload->member = test_val;
  test_nest_s data_source = {.nested = payload};
  result = libd_platform_thread_local_storage_set(handle, _test_nest_setter_f,
                                                  &data_source);
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));

  test_nest_s data_dest = {.nested = NULL};
  test_nest_s* p_data_dest = &data_dest;
  result = libd_platform_thread_local_storage_get(handle, (void**)&p_data_dest);
  cr_assert(eq(u8, p_data_dest->nested->member, test_val));

  result = libd_platform_thread_local_storage_destroy(handle);
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));
}

Test(static, init)
{
  libd_platform_thread_result_e result =
    libd_platform_thread_local_static_init(free, sizeof(test_s));
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));
}

Test(static, cleanup)
{
  libd_platform_thread_result_e result =
    libd_platform_thread_local_static_init(free, sizeof(test_s));
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));

  result = libd_platform_thread_local_static_cleanup();
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));
}

Test(static, set_get_flat)
{
  int test_val = 10;
  libd_platform_thread_result_e result =
    libd_platform_thread_local_static_init(free, sizeof(test_s));
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));

  test_s src = {.member = test_val};
  result = libd_platform_thread_local_static_set(NULL, &src);
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));

  src.member = 0;
  test_s* dest = &src;
  result = libd_platform_thread_local_static_get((void**)&dest);
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));
  cr_assert(eq(u8, dest->member, test_val));

  result = libd_platform_thread_local_static_cleanup();
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));
}
Test(static, set_get_nested)
{
  int test_val = 10;
  libd_platform_thread_result_e result = libd_platform_thread_local_static_init(
    _test_nest_destructor_f, sizeof(test_s));
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));

  test_s* nested = malloc(sizeof(test_s));
  cr_assert(ne(ptr, nested, NULL));
  nested->member = test_val;
  test_nest_s src_data = {.nested = nested};
  result =
    libd_platform_thread_local_static_set(_test_nest_setter_f, &src_data);
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));

  test_nest_s* dest_data = &src_data;
  dest_data->nested = NULL;
  result = libd_platform_thread_local_static_get((void**)&dest_data);
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));
  cr_assert(eq(u8, dest_data->nested->member, test_val));

  result = libd_platform_thread_local_static_cleanup();
  cr_assert(eq(u8, result, LIBD_PF_THREAD_OK));
}
