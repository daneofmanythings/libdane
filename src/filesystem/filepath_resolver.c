#include "../../include/libd/platform/filesystem.h"
#include "../../include/libd/utils/align_compat.h"
#include "filepath.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum libd_result
libd_filepath_resolver_init(
  struct filepath_resolver** out_pt,
  const char* src_path,
  enum libd_filesystem_path_type path_type)
{
  enum libd_result r = libd_ok;

  struct filepath_resolver* fpr = malloc(sizeof(*fpr));
  if (fpr == NULL) {
    return libd_no_memory;
  }

  fpr->src       = src_path;
  fpr->path_type = path_type;

  r = libd_filepath_allocator_create(
    &fpr->allocator, LIBD_ALIGNOF(struct path_token_node));
  if (r != libd_ok) {
    free(fpr);
    return libd_no_memory;
  }

  *out_pt = fpr;

  return r;
}

void
libd_filepath_resolver_release(struct filepath_resolver* pt)
{
  libd_filepath_allocator_destroy(&pt->allocator);
  free(pt);
}

static struct path_token_node*
libd_filepath_resolver_make_token_node(struct filepath_resolver* fpr);
enum libd_result
libd_filepath_resolver_tokenize(struct filepath_resolver* fpr)
{
  fpr->allocator.reset(&fpr->allocator.wrapper);
  fpr->scan_pos = 0;

  struct path_token_node* node;
  do {
    node = libd_filepath_resolver_make_token_node(fpr);

    node->prev      = fpr->head;
    fpr->head->next = node;
    fpr->head       = node;

  } while (node->token.type != eof_type);

  return libd_ok;
}

static struct path_token_node*
libd_filepath_resolver_make_separator_token(struct filepath_resolver* fpr);
static struct path_token_node*
libd_filepath_resolver_make_eof_token(struct filepath_resolver* fpr);
static struct path_token_node*
libd_filepath_resolver_make_component_token(struct filepath_resolver* fpr);

static struct path_token_node*
libd_filepath_resolver_make_token_node(struct filepath_resolver* fpr)
{
  switch (fpr->src[fpr->scan_pos]) {
  case PATH_SEPARATOR:
    return libd_filepath_resolver_make_separator_token(fpr);
    break;
  case NULL_TERMINATOR:
    return libd_filepath_resolver_make_eof_token(fpr);
    break;
  default:
    return libd_filepath_resolver_make_component_token(fpr);
  }
}

static struct path_token_node*
libd_filepath_resolver_make_separator_token(struct filepath_resolver* fpr)
{
  struct path_token_node* node = (struct path_token_node*)fpr->allocator.alloc(
    &fpr->allocator.wrapper, sizeof(*node) + PATH_SEPARATOR_VALUE_LEN);

  node->token.type = separator_type;
  memcpy(node->token.value, PATH_SEPARATOR_VALUE, PATH_SEPARATOR_VALUE_LEN);

  fpr->scan_pos += 1;

  return node;
}

static struct path_token_node*
libd_filepath_resolver_make_eof_token(struct filepath_resolver* fpr)
{
  struct path_token_node* node = (struct path_token_node*)fpr->allocator.alloc(
    &fpr->allocator.wrapper, sizeof(*node) + 1);

  node->token.type     = eof_type;
  node->token.value[0] = NULL_TERMINATOR;

  return node;
}

static struct path_token_node*
libd_filepath_resolver_make_component_token(struct filepath_resolver* fpr)
{
  size_t component_start = fpr->scan_pos;
  while (fpr->src[fpr->scan_pos] != PATH_SEPARATOR &&
         fpr->src[fpr->scan_pos] != NULL_TERMINATOR) {
    fpr->scan_pos += 1;
  }
  size_t component_length = fpr->scan_pos - component_start;

  struct path_token_node* node = (struct path_token_node*)fpr->allocator.alloc(
    &fpr->allocator.wrapper, sizeof(*node) + component_length + 1);

  node->token.type = component_type;
  memcpy(node->token.value, &fpr->src[component_start], component_length);
  node->token.value[component_length] = NULL_TERMINATOR;

  return node;
}
