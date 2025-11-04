#include "../include/libdane/errors.h"

#include "internal/internal.h"

#include <stdarg.h>
#include <stdio.h>

enum libd_result
libd_error_err_get(struct libd_error_context** pp_err)
{
  return platform_thread_local_static_get(
    (void**)pp_err, sizeof(struct libd_error_context));
}

enum libd_result
libd_errors_err_set(
  int code,
  const char* fmt,
  ...)
{
  struct libd_error_context new_err = { .code = 0, .msg = "" };

  new_err.code = code;

  va_list args;
  va_start(args, fmt);
  vsnprintf(new_err.msg, sizeof(new_err.msg), fmt, args);
  va_end(args);

  return platform_thread_local_static_set(
    &new_err, sizeof(struct libd_error_context));
}
