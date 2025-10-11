#include "../include/libdane/errors.h"
#include "internal/internal.h"
#include <stdarg.h>
#include <stdio.h>


libd_errors_result_e
libd_error_err_init(void)
{
  return platform_thread_local_static_init(sizeof(libd_errors_err_s));
}

libd_errors_result_e
libd_error_err_get(libd_errors_err_s** pp_err)
{
  return platform_thread_local_static_get((void**)pp_err,
                                          sizeof(libd_errors_err_s));
}

libd_errors_result_e
libd_errors_err_set(int code, const char* fmt, ...)
{
  libd_errors_err_s* new_err;

  new_err->code = code;

  va_list args;
  va_start(args, fmt);
  vsnprintf(new_err->msg, sizeof(new_err->msg), fmt, args);
  va_end(args);

  return platform_thread_local_static_set(new_err, sizeof(libd_errors_err_s));
}
