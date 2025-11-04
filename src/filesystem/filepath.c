#include "filepath.h"

#include "../../include/libd/filesystem.h"

#include <string.h>

enum libd_result
libd_filesystem_path_init(
  struct filepath* out_fp,
  const char* raw_path,
  enum libd_filesystem_path_type type,
  libd_filesystem_env_getter_f env_getter)
{
  if (out_fp == NULL || raw_path == NULL) {
    return libd_invalid_parameter;
  }

  enum libd_result r;
  struct filepath_resolver* fpr;

  r = libd_filepath_resolver_init(&fpr, raw_path, type);
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

  // TODO: init the relevant filepath fields, mainly the allocator.
  r = libd_filepath_allocator_create(&out_fp->allocator, 1);
  if (r != 0) {
    goto cleanup;
  }

  r = libd_filepath_resolver_dump(fpr, out_fp);
  if (r != 0) {
    goto cleanup;
  }

cleanup:
  libd_filepath_resolver_release(fpr);
  return r;
}
