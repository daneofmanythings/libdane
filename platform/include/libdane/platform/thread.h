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
  LIBD_PF_THREAD_OK,        /**< Operation succeeded */
  LIBD_PF_THREAD_NO_MEMORY, /**< Could not allocate memory for the operation */
  LIBD_PF_THREAD_NULL_PARAMETER,  /**< Recieved an unexpected null pointer as
                         a parameter */
  LIBD_PF_THREAD_INIT_FAILED,     /**< Initialization of the singleton failed */
  LIBD_PF_THREAD_NOT_INITIALIZED, /**< Dependent functions called before
                                     initialization */
  LIBD_PLATFORM_THREAD_RESULT_E_COUNT, /**< Total number of result codes */
} libd_platform_thread_result_e;

//==============================================================================
// Type Definitions
//==============================================================================

/**
 * @brief Function pointer type for setting thread local storage data
 * @warning Care must be taken when deciding how the allocated memory is handled
 * and how it is set/cleaned.
 * @param arg1 First argument (context or old data)
 * @param arg2 Second argument (new data)
 * @return 0 on success, non-zero on failure
 */
typedef int (*libd_platform_thread_local_storage_data_setter_f)(void*, void*);

/**
 * @brief The destructor to register for the data being stored. Is called
 * automatically upon thread key destruction.
 * @param arg1 Void pointer to the stored data
 */
typedef void (*libd_platform_thread_local_storage_destructor_f)(void*);

/**
 * @brief Opaque handle for thread local storage
 */
typedef struct libd_platform_thread_local_storage_handle_s
  libd_platform_thread_local_storage_handle_s;

//==============================================================================
// Thread Local Storage API
//==============================================================================

/**
 * @brief Creates thread local storage handle once (thread-safe singleton)
 * @param pp_handle Pointer to receive the handle
 * @return RESULT_OK on success, error code otherwise
 */
libd_platform_thread_result_e
libd_platform_thread_local_storage_create(
  libd_platform_thread_local_storage_handle_s** p_handle,
  libd_platform_thread_local_storage_destructor_f destructor,
  size_t size);

/**
 * @brief Destroys thread local storage associated with the provided handle
 * @param p_handle Handle to the data to destroy
 */
libd_platform_thread_result_e
libd_platform_thread_local_storage_destroy(
  libd_platform_thread_local_storage_handle_s* handle);

/**
 * @brief Gets data from thread local storage
 * @param p_handle Storage handle
 * @param data Pointer to receive the data
 * @return RESULT_OK on success, error code otherwise
 */
libd_platform_thread_result_e
libd_platform_thread_local_storage_get(
  libd_platform_thread_local_storage_handle_s* handle,
  void** data);

/**
 * @brief Sets data in thread local storage.
 * @param p_handle Storage handle.
 * @param setter Function to set the data.
 * @param new_data New data to store.
 * @return RESULT_OK on success, error code otherwise.
 */
libd_platform_thread_result_e
libd_platform_thread_local_storage_set(
  libd_platform_thread_local_storage_handle_s* handle,
  libd_platform_thread_local_storage_data_setter_f setter,
  void* new_data);

/**
 * @brief Initializes the singleton thread-local storage slot (idempotent).
 * @warning Calling init from multiple threads is undefined behavior. Call init
 * first. Note that this API commits the calling process to the registered data
 * size and destructor.
 * @param destructor Registered callback to clean up stored data.
 * @param size The size to allocate.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_platform_thread_result_e
libd_platform_thread_local_static_init(
  libd_platform_thread_local_storage_destructor_f destructor,
  size_t size);

/**
 * @brief Gets the data in the singleton thread-local slot
 * @param pp_data Pointer to a handle for the data
 * @return RESULT_OK, non-zero otherwise.
 */
libd_platform_thread_result_e
libd_platform_thread_local_static_get(void** pp_data);

/**
 * @brief Sets the stored value or applies setter to mutate the stored value.
 * @param setter An optional function to mutate the stored data with; *p_data is
 * copied to storage if NULL.
 * @param p_data handle to the new data.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_platform_thread_result_e
libd_platform_thread_local_static_set(
  libd_platform_thread_local_storage_data_setter_f setter,
  void* p_data);

/**
 * @brief Cleans up the initialized singleton thread-local storage slot.
 */
libd_platform_thread_result_e
libd_platform_thread_local_static_cleanup(void);

#endif  // LIBD_PLATFORM_THREAD_H
