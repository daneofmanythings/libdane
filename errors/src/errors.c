#include "../include/libdane/errors.h"
#include "internal/internal.h"

struct dlib_error_err_s {
  int code;
  char msg[512];
};

libd_errors_result_e
libd_error_err_get(dlib_error_err_s** pp_err)
{
  int result = platform_thread_local_storage_create_once();
  if (result != 0) {
    // TODO:
  }

  result =
    platform_thread_local_storage_get((void**)pp_err, sizeof(dlib_error_err_s));
  if (result != RESULT_OK) {
    // TODO:
  }

  return RESULT_OK;
}

libd_errors_result_e
libd_errors_err_set(int code, const char* fmt, ...)
{
  int err = platform_thread_local_storage_create_once();
  if (err != 0) {
    return err;
  }

  dlib_error_err_s new_data = {0};
  // TODO: va_args into new data

  err = platform_thread_local_storage_set(&new_data, sizeof(dlib_error_err_s));
  if (err != 0) {
    return err;
  }

  return RESULT_OK;
}
