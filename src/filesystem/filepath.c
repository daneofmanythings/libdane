#include "filepath.h"

#include "../../include/libd/filesystem.h"

#include <string.h>

size_t
libd_filesystem_filepath_get_required_size(void)
{
  return sizeof(struct filepath);
}

enum libd_result
libd_filesystem_filepath_init(
  struct filepath* out_fp,
  const char* raw_path,
  enum libd_filesystem_path_type type,
  libd_filesystem_env_get_f env_getter)
{
  if (out_fp == NULL || raw_path == NULL || raw_path[0] == NULL_TERMINATOR) {
    return libd_invalid_parameter;
  }

  enum libd_result r;
  struct filepath_resolver* fpr;

  r = libd_filepath_resolver_create(&fpr, raw_path, type);
  if (r != 0) {
    return r;
  }

  r = libd_filepath_resolver_tokenize(fpr);
  if (r != 0) {
    goto cleanup;
  }

  r = libd_filepath_resolver_expand(fpr, env_getter);
  if (r != 0) {
    goto cleanup;
  }

  r = libd_filepath_resolver_normalize(fpr);
  if (r != 0) {
    goto cleanup;
  }

  r = libd_filepath_allocator_create(&out_fp->allocator, 1);
  if (r != 0) {
    goto cleanup;
  }
  out_fp->length = 0;

  r = libd_filepath_resolver_dump_to_filepath(fpr, out_fp);
  if (r != 0) {
    goto cleanup;
  }

cleanup:
  libd_filepath_resolver_destroy(fpr);
  return r;
}

enum libd_result
libd_filesystem_filepath_string(
  libd_filepath_h* path,
  char* out_string)
{
  char* writer = out_string;
  u8 i         = 0;
  while (!IS_EOF_TYPE(path->types[i])) {
    if (i >= FILEPATH_COMPONENT_MAX) {
      return libd_no_memory;  // FIX: this is not expressive
    }
    strcpy(writer, *path->values);
    writer += strlen(*path->values);
    i += 1;
  }

  return libd_ok;
}
