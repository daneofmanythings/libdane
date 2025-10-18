/**
 * @file filesystem.h
 * @brief A set of platform agnostic utilities for working with the filesystem.
 */

#include <libdane/platform/filesystem.h>

//==============================================================================
// Result Codes
//==============================================================================

/**
 * @brief The result codes used by this library.
 */
typedef enum {
  RESULT_OK,                      /**< Operation successful */
  ERR_INVALID_PATH,               /**< The path provided is invalid */
  LIBD_FILESYSTEM_RESULT_E_COUNT, /**< The number of result codes */
} libd_filesystem_result_e;

//==============================================================================
// Type definitions
//==============================================================================

/**
 * @brief Opaque type for libd_filesystem_path_s. Is used for path operations
 * with a safe, managed abstraction.
 */
typedef struct libd_filesystem_path_s libd_filesystem_path_o;

//==============================================================================
// Path API
//==============================================================================

/**
 * @brief Initializes a path_s.
 * @param out_filepath Out parameter for the created path.
 * @param path_string Path to initialize from.
 * @return RESULT_OK on success, non-zero otherwise.
 */
libd_filesystem_result_e
libd_filesystem_path_create(libd_filesystem_path_o** out_path,
                            const char* path_string);

//
libd_filesystem_result_e
libd_filesystem_expand_env_vars(char dest[PATH_MAX], const char* path);
libd_filesystem_result_e
write_new_file(const char* path, const char* body);
libd_filesystem_result_e
write_to_scratch(const char* path, const char* body);
libd_filesystem_result_e
mkdir_from_absolute_filepath(const char* filepath);
