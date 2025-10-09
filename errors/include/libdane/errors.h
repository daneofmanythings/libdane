#ifndef LIBDANE_ERRORS_H
#define LIBDANE_ERRORS_H

typedef enum {
  RESULT_OK,
  LIBD_ERRORS_RESULT_E_COUNT,
} libd_errors_result_e;

typedef struct dlib_error_err_s dlib_error_err_s;

libd_errors_result_e
libd_error_err_get(dlib_error_err_s** pp_err);
libd_errors_result_e
libd_errors_err_set(int code, const char* fmt, ...);

#endif  // LIBDANE_ERRORS_H
