/**
 * @file errors.h
 * @brief Error handling with string context.
 */

#ifndef LIBDANE_ERRORS_H
#define LIBDANE_ERRORS_H


//==============================================================================
// Result Codes
//==============================================================================

/**
 * @brief Result codes. Note that return values are being inherited from the
 * platform library. This is not intended, just a side effect of 'good enough.'
 * I will get around to fixing it eventually.
 */
#include <stddef.h>
typedef enum {
  LIBD_ERR_OK,
  LIBD_ERRORS_RESULT_E_COUNT,
} libd_errors_result_e;


//==============================================================================
// Type Definitions
//==============================================================================

/**
 * @brief Struct to hold the error context.
 */
typedef struct {
  int code;      /**< User defined error code */
  char msg[512]; /**< Human readable error context */
} libd_errors_err_s;


//==============================================================================
// Error handling API
//==============================================================================

/**
 * @brief Gets the error context and places it in pp_err.
 * @param pp_err Pointer to the error handle.
 * @return RESULT_OK on success, error code otherwise
 */
libd_errors_result_e
libd_error_err_get(libd_errors_err_s** pp_err);


/**
 * @brief Sets the error code and string context.
 * @param code The user defined error code.
 * @param fmt The string template to use.
 * @param ... The variadic arguements to put into the string template.
 * @return RESULT_OK on success, error code otherwise
 */
libd_errors_result_e
libd_errors_err_set(int code, const char* fmt, ...);

#endif  // LIBDANE_ERRORS_H
