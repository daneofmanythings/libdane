#include "filepath.h"

#include "../../include/libd/filesystem.h"
#include "../../include/libd/platform/filesystem.h"

#include <string.h>

size_t
libd_filesystem_filepath_get_required_size(void)
{
  return sizeof(struct filepath);
}

enum libd_result
libd_filesystem_filepath_init(
  struct filepath* fp,
  const char* raw_path,
  enum libd_filesystem_path_type type,
  libd_filesystem_env_get_f env_getter)
{
  if (fp == NULL || raw_path == NULL || raw_path[0] == NULL_TERMINATOR) {
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

  if (env_getter != NULL) {
    r = libd_filepath_resolver_expand(fpr, env_getter);
    if (r != 0) {
      goto cleanup;
    }
  }

  r = libd_filepath_resolver_normalize(fpr);
  if (r != 0) {
    goto cleanup;
  }

  r = libd_filepath_allocator_create(&fp->allocator, 1);
  if (r != 0) {
    goto cleanup;
  }
  fp->length = 0;

  r = libd_filepath_resolver_dump_to_filepath(fpr, fp);
  if (r != 0) {
    goto cleanup;
  }

cleanup:
  libd_filepath_resolver_destroy(fpr);
  return r;
}

enum libd_result
libd_filesystem_filepath_string(
  struct filepath* fp,
  char* out_string)
{
  char* writer = out_string;
  if (!libd_filepath_is_abs(fp->path_type)) {
    *writer++ = '.';
    *writer++ = PATH_SEPARATOR;
  }
  u8 i = 0;
  while (!IS_EOF_TYPE(fp->types[i])) {
    if (i >= FILEPATH_COMPONENT_MAX) {
      return libd_err;  // FIX: this is not expressive
    }
    strcpy(writer, fp->values[i]);
    writer += strlen(fp->values[i]);
    i += 1;
  }

  return libd_ok;
}
