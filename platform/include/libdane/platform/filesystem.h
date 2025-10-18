/**
 * @file filesystem.h
 * @brief A set of utilities for dealing with the filesystem.
 */

//==============================================================================
// Result Codes
//==============================================================================

/**
 * @brief The result codes used by this library.
 */
#include <stddef.h>
typedef enum {
  LIBD_PF_FS_OK,                           /**< Operation successful */
  LIBD_PF_FS_INVALID_PATH,                 /**< The path provided is invalid */
  LIBD_PLATFORM_FILESYSTEM_RESULT_E_COUNT, /**< The number of result codes */
} libd_platform_filesystem_result_e;

//==============================================================================
// Type definitions
//==============================================================================

/**
 * @brief Opaque handle to the file handle/descriptor.
 */
typedef struct libd_platform_filesystem_file_handle_s
  libd_platform_filesystem_file_handle_o;

/**
 * @brief Opens a file on the system, returning the handle through an out
 * parameter.
 * @param out_handle Out paraneter for the handle.
 * @param path Path to the file.
 * @param path_length Length of the path_string.
 * @param mode The mode to open the file in.
 * @returns LIBD_PF_FS_OK if success, non-zero otherwise.
 */
libd_platform_filesystem_result_e
libd_platform_filesystem_file_open(
  // TODO: Examine the allocation strategy
  libd_platform_filesystem_file_handle_o** out_handle,
  const char* path,
  const size_t path_length,
  const char* mode);

// fs_close(FS_File *file); — Close a file handle.
libd_platform_filesystem_result_e
libd_platform_filesystem_file_close(
  libd_platform_filesystem_file_handle_o* out_handle);
//
// fs_read(void *buffer, size_t size, size_t count, FS_File *file); — Read
// from a file.
//
// fs_write(const void *buffer, size_t size, size_t count, FS_File *file); —
// Write to a file.
//
// fs_seek(FS_File *file, long offset, int origin); — Move file pointer.
//
// fs_tell(FS_File *file); — Get current file position.
//
// fs_exists(const char *path); — Check if a file exists.
//
// fs_remove(const char *path); — Delete a file.
//
// fs_copy(const char *src, const char *dst); — Copy a file cross-platform.
//
// fs_rename(const char *oldpath, const char *newpath); — Rename or move a
// file.
//
// 3. Directory Management
//
// fs_mkdir(const char *path); — Create a directory (recursively if needed).
//
// fs_rmdir(const char *path); — Remove a directory.
//
// fs_listdir(const char *path, FS_DirEntryCallback callback); — Enumerate
// directory contents.
//
// fs_is_dir(const char *path); — Check if path is a directory.
//
// fs_is_file(const char *path); — Check if path is a regular file.
//
// 4. Path Utilities
//
// fs_join_path(char *out, size_t out_size, const char *a, const char *b); —
// Join two path segments safely.
//
// fs_normalize_path(char *path); — Normalize path separators and relative
// segments.
//
// fs_get_filename(const char *path); — Extract filename from a path.
//
// fs_get_extension(const char *path); — Get file extension.
//
// fs_get_parent_dir(const char *path); — Retrieve parent directory.
//
// 5. Metadata and Attributes
//
// fs_stat(const char *path, FS_Stat *out); — Retrieve file metadata (size,
// timestamps, etc.).
//
// fs_set_permissions(const char *path, FS_Permissions perms); — Set file
// permissions in a portable way.
//
// fs_get_permissions(const char *path, FS_Permissions *out); — Query
// permissions.
//
// fs_get_timestamp(const char *path, FS_TimestampType type); — Retrieve
// timestamps (created, modified, accessed).
//
// 6. Abstraction and Portability
//
// fs_translate_path(const char *path_in, char *path_out, size_t out_size); —
// Convert between platform path conventions.
//
// fs_is_absolute(const char *path); — Determine if path is absolute on the
// target platform.
//
// fs_get_temp_dir(char *out, size_t out_size); — Get platform’s temporary
// directory path.
//
// fs_get_home_dir(char *out, size_t out_size); — Get user’s home directory
// path.
//
// 7. Advanced / Optional
//
// fs_mount(const char *virtual_path, const char *real_path); — Mount virtual
// paths for sandboxed or packaged filesystems.
//
// fs_unmount(const char *virtual_path); — Unmount a virtual path.
//
// fs_get_free_space(const char *path, uint64_t *out_bytes); — Query available
// disk space.
//
// fs_sync(FS_File *file); — Force data flush to disk.
//
// fs_watch(const char *path, FS_FileWatchCallback callback);
