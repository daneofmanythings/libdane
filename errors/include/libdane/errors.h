/**
 * @file errors.h
 * @brief Error handling with string context.
 */

#ifndef LIBDANE_ERRORS_H
#define LIBDANE_ERRORS_H

#include "../../../include/types.h"

#include <stddef.h>

//==============================================================================
// Type Definitions
//==============================================================================

/**
 * @brief Struct to hold the error context.
 */
struct libd_error_context {
  int code;      /**< User defined error code */
  char msg[512]; /**< Human readable error context */
};

//==============================================================================
// Error handling API
//==============================================================================

/**
 * @brief Gets the error context and places it in pp_err.
 * @param pp_err Pointer to the error handle.
 * @return RESULT_OK on success, error code otherwise
 */
enum libd_result
libd_error_err_get(struct libd_error_context** pp_err);

/**
 * @brief Sets the error code and string context.
 * @param code The user defined error code.
 * @param fmt The string template to use.
 * @param ... The variadic arguements to put into the string template.
 * @return RESULT_OK on success, error code otherwise
 */
enum libd_result
libd_errors_err_set(
  int code,
  const char* fmt,
  ...);

#endif  // LIBDANE_ERRORS_H
