/**
 * @file thread.h
 * @brief Platform-specific thread local storage interface
 */

#ifndef LIBD_PLATFORM_THREAD_H
#define LIBD_PLATFORM_THREAD_H

#include <stddef.h>

//==============================================================================
// Result Codes
//==============================================================================

/**
 * @brief Result codes for thread operations
 */
typedef enum {
  RESULT_OK,                    /**< Operation succeeded */
  LIBD_PLATFORM_RESULT_E_COUNT, /**< Total number of result codes */
} libd_platform_thread_result_e;

//==============================================================================
// Type Definitions
//==============================================================================

/**
 * @brief Function pointer type for setting thread local storage data
 * @param arg1 First argument (context or old data)
 * @param arg2 Second argument (new data)
 * @return 0 on success, non-zero on failure
 */
typedef int (*libd_platform_thread_local_storage_data_setter_f)(void*, void*);

/**
 * @brief Opaque handle for thread local storage
 */
typedef struct libd_platform_thread_local_storage_handle_s
  libd_platform_thread_local_storage_handle_s;

//==============================================================================
// Thread Local Storage API
//==============================================================================

/**
 * @brief Creates a new thread local storage handle
 * @param p_handle Pointer to receive the created handle
 * @return RESULT_OK on success, error code otherwise
 */
libd_platform_thread_result_e
libd_platform_thread_local_storage_handle_create(
  libd_platform_thread_local_storage_handle_s** p_handle);

/**
 * @brief Destroys a thread local storage handle
 * @param handle Handle to destroy
 */
void
libd_platform_thread_local_storage_handle_destroy(
  libd_platform_thread_local_storage_handle_s* handle);

/**
 * @brief Creates thread local storage handle once (thread-safe singleton)
 * @param pp_handle Pointer to receive the handle
 * @return RESULT_OK on success, error code otherwise
 */
libd_platform_thread_result_e
libd_platform_thread_local_storage_create_once(
  libd_platform_thread_local_storage_handle_s** pp_handle);

/**
 * @brief Destroys thread local storage
 * @param p_handle Handle to destroy
 */
void
libd_platform_thread_local_storage_destroy(
  libd_platform_thread_local_storage_handle_s* p_handle);

/**
 * @brief Gets data from thread local storage
 * @param p_handle Storage handle
 * @param data Pointer to receive the data
 * @param size Size of data buffer
 * @return RESULT_OK on success, error code otherwise
 */
libd_platform_thread_result_e
libd_platform_thread_local_storage_get(
  libd_platform_thread_local_storage_handle_s* p_handle,
  void** data,
  size_t size);

/**
 * @brief Sets data in thread local storage
 * @param p_handle Storage handle
 * @param setter Function to set the data
 * @param new_data New data to store
 * @param size Size of data
 * @return RESULT_OK on success, error code otherwise
 */
libd_platform_thread_result_e
libd_platform_thread_local_storage_set(
  libd_platform_thread_local_storage_handle_s* p_handle,
  libd_platform_thread_local_storage_data_setter_f setter,
  void* new_data,
  size_t size);

#endif  // LIBD_PLATFORM_THREAD_H
