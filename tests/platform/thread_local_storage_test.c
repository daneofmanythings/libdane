#include "../../include/libd/common.h"
#include "../../include/libd/platform/threads.h"
#include "../../include/libd/testing.h"

#include <stdbool.h>

struct flat_storage {
  int member;
};

struct nested_storage {
  struct flat_storage* payload;
};

libd_platform_thread_local_storage_handle_h*
helper_create_thread_local_storage(void)
{
  libd_platform_thread_local_storage_handle_h* handle;
  ASSERT_OK(libd_platform_thread_local_storage_create(
    &handle, free, sizeof(struct flat_storage)));

  return handle;
}

TEST(thread_local_storage_flat_set_get)
{
  int test_val = 10;
  libd_platform_thread_local_storage_handle_h* handle;
  ASSERT_OK(libd_platform_thread_local_storage_create(
    &handle, free, sizeof(struct flat_storage)));

  struct flat_storage data_source = { .member = test_val };
  ASSERT_OK(libd_platform_thread_local_storage_set(handle, NULL, &data_source));

  struct flat_storage data_dest    = { .member = 0 };
  struct flat_storage* p_data_dest = &data_dest;
  ASSERT_OK(
    libd_platform_thread_local_storage_get(handle, (void**)&p_data_dest));
  ASSERT_EQ_U(p_data_dest->member, test_val);

  ASSERT_OK(libd_platform_thread_local_storage_destroy(handle));
}

static int
_test_nest_setter_f(
  void* dest,
  void* src)
{
  struct nested_storage* t_dest = (void*)dest;
  struct nested_storage* t_src  = (void*)src;

  t_dest->payload = t_src->payload;

  return libd_ok;
}
static void
_test_nest_destructor_f(void* dat)
{
  struct nested_storage* t = (struct nested_storage*)dat;
  if (t->payload != NULL) {
    free(t->payload);
  }
}

TEST(thread_local_storage_nested_set_get)
{
  int test_val = 10;
  libd_platform_thread_local_storage_handle_h* handle;
  ASSERT_OK(libd_platform_thread_local_storage_create(
    &handle, _test_nest_destructor_f, sizeof(struct nested_storage)));

  struct flat_storage* payload = malloc(sizeof(struct flat_storage));
  ASSERT_NOT_NULL(payload);
  payload->member                   = test_val;
  struct nested_storage data_source = { .payload = payload };

  ASSERT_OK(libd_platform_thread_local_storage_set(
    handle, _test_nest_setter_f, &data_source));

  struct nested_storage data_dest    = { .payload = NULL };
  struct nested_storage* p_data_dest = &data_dest;
  ASSERT_OK(
    libd_platform_thread_local_storage_get(handle, (void**)&p_data_dest));
  ASSERT_EQ_U(p_data_dest->payload->member, test_val);

  ASSERT_OK(libd_platform_thread_local_storage_destroy(handle));
}

TEST(thread_local_storage_global_state_init_cleanup)
{
  ASSERT_OK(
    libd_platform_thread_local_static_init(free, sizeof(struct flat_storage)));
  ASSERT_OK(libd_platform_thread_local_static_cleanup());
}

TEST(thread_local_static_flat_get_set)
{
  int test_val = 10;
  ASSERT_OK(
    libd_platform_thread_local_static_init(free, sizeof(struct flat_storage)));

  struct flat_storage src = { .member = test_val };
  ASSERT_OK(libd_platform_thread_local_static_set(NULL, &src));

  src.member                = 0;
  struct flat_storage* dest = &src;
  ASSERT_OK(libd_platform_thread_local_static_get((void**)&dest));
  ASSERT_EQ_U(dest->member, test_val);

  ASSERT_OK(libd_platform_thread_local_static_cleanup());
}

TEST(thread_local_static_nested_get_set)
{
  int test_val = 10;
  ASSERT_OK(libd_platform_thread_local_static_init(
    _test_nest_destructor_f, sizeof(struct nested_storage)));

  struct flat_storage* flat = malloc(sizeof(struct flat_storage));
  ASSERT_NOT_NULL(flat);
  flat->member = test_val;

  struct nested_storage src_data = { .payload = flat };
  ASSERT_OK(
    libd_platform_thread_local_static_set(_test_nest_setter_f, &src_data));

  struct nested_storage* dest_data = &src_data;
  dest_data->payload               = NULL;
  ASSERT_OK(libd_platform_thread_local_static_get((void**)&dest_data));
  ASSERT_EQ_U(dest_data->payload->member, test_val);

  ASSERT_OK(libd_platform_thread_local_static_cleanup());
}
