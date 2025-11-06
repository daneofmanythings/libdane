#include "../../include/libd/platform/filesystem.h"
#include "../../include/libd/utils/align_compat.h"
#include "./filepath.h"
#include "./internal/platform_wrap.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

enum libd_result
libd_filepath_resolver_create(
  struct filepath_resolver** out_fpr,
  const char* src_path,
  enum libd_filesystem_path_type path_type)
{
  if (out_fpr == NULL || src_path == NULL || src_path[0] == NULL_TERMINATOR) {
    return libd_invalid_parameter;
  }
  enum libd_result r = libd_ok;

  struct filepath_resolver* fpr = malloc(sizeof(*fpr));
  if (fpr == NULL) {
    return libd_no_memory;
  }

  fpr->src       = src_path;
  fpr->path_type = path_type;
  fpr->head      = NULL;

  r = libd_filepath_allocator_create(
    &fpr->allocator, LIBD_ALIGNOF(struct path_token_node));
  if (r != libd_ok) {
    free(fpr);
    return libd_no_memory;
  }

  *out_fpr = fpr;

  return r;
}

void
libd_filepath_resolver_destroy(struct filepath_resolver* pt)
{
  libd_filepath_allocator_destroy(&pt->allocator);
  free(pt);
}

struct path_token_node*
libd_tokenize_from_string(
  const char* src,
  struct filepath_allocator* fpa);
enum libd_result
libd_filepath_resolver_tokenize(struct filepath_resolver* fpr)
{
  fpr->allocator.reset(&fpr->allocator.wrapper);
  fpr->head = libd_tokenize_from_string(fpr->src, &fpr->allocator);

  return libd_ok;
}

static struct path_token_node*
libd_make_path_token_node(
  const char* src,
  size_t scan_pos,
  struct filepath_allocator* fpa);
struct path_token_node*

libd_tokenize_from_string( // FIX: this needs error handling
  const char* src,
  struct filepath_allocator* fpa)
{
  size_t scan_pos = 0;

  struct path_token_node* head = NULL;
  struct path_token_node* tail;
  struct path_token_node* node;

  do {
    node = libd_make_path_token_node(src, scan_pos, fpa);

    if (head == NULL) {
      head = node;
      tail = node;
    } else {
      node->prev = tail;
      tail->next = node;
      tail       = node;
    }

    scan_pos += node->val_len;

  } while (node->type != eof_type);

  return head;
}

static struct path_token_node*
libd_make_separator_token(struct filepath_allocator* fpa);
static struct path_token_node*
libd_make_eof_token(struct filepath_allocator* fpa);
static struct path_token_node*
libd_make_component_token(
  const char* src,
  size_t scan_pos,
  struct filepath_allocator* fpa);
static struct path_token_node*

libd_make_path_token_node( // FIX: this needs error handling
  const char* src,
  size_t scan_pos,
  struct filepath_allocator* fpa)
{
  switch (src[scan_pos]) {
  case PATH_SEPARATOR:
    return libd_make_separator_token(fpa);
  case NULL_TERMINATOR:
    return libd_make_eof_token(fpa);
  default:
    return libd_make_component_token(src, scan_pos, fpa);
  }
}

static struct path_token_node*
libd_make_separator_token(struct filepath_allocator* fpa)
{
  struct path_token_node* node = (struct path_token_node*)fpa->alloc(
    &fpa->wrapper, sizeof(*node) + PATH_SEPARATOR_VALUE_LEN);

  node->prev     = NULL;
  node->next     = NULL;
  node->type     = separator_type;
  node->value[0] = PATH_SEPARATOR;
  node->value[1] = NULL_TERMINATOR;
  node->val_len  = 1;

  return node;
}

static struct path_token_node*
libd_make_eof_token(struct filepath_allocator* fpa)
{
  struct path_token_node* node =
    (struct path_token_node*)fpa->alloc(&fpa->wrapper, sizeof(*node) + 1);

  node->prev     = NULL;
  node->next     = NULL;
  node->type     = eof_type;
  node->value[0] = NULL_TERMINATOR;
  node->val_len  = 0;

  return node;
}

static struct path_token_node*
libd_make_component_token(
  const char* src,
  size_t scan_pos,
  struct filepath_allocator* fpa)
{
  size_t component_start = scan_pos;
  while (src[scan_pos] != PATH_SEPARATOR && src[scan_pos] != NULL_TERMINATOR) {
    scan_pos += 1;
  }
  size_t component_length = scan_pos - component_start;

  struct path_token_node* node = (struct path_token_node*)fpa->alloc(
    &fpa->wrapper, sizeof(*node) + component_length + 1);

  node->prev = NULL;
  node->next = NULL;
  node->type = component_type;
  memcpy(node->value, &src[component_start], component_length);
  node->value[component_length] = NULL_TERMINATOR;
  node->val_len                 = component_length;

  return node;
}

enum libd_result
libd_filepath_resolver_expand( // FIX: error handling
  struct filepath_resolver* fpr,
  libd_filesystem_env_get_f env_get)
{
  struct path_token_node* curr_node = fpr->head;
  struct path_token_node* temp      = NULL;

  char env_val[256] = { 0 };

  enum libd_result result = libd_ok;

  while (curr_node->type != eof_type) {
    bool is_env_var = platform_is_env_var(curr_node->value);

    if (!is_env_var) {
      curr_node = curr_node->next;
      continue;
    }

    result = platform_env_var_get(env_val, curr_node->value, env_get);
    if (result != libd_ok) {
      break;
    }

    struct path_token_node* new_node =
      libd_tokenize_from_string(env_val, &fpr->allocator);

    if (curr_node->prev) {
      curr_node->prev->next = new_node;
    }
    new_node->prev = curr_node->prev;

    temp = new_node;
    if (new_node->next != NULL) {
      while (new_node->next->type != eof_type) {
        new_node = new_node->next;
      }
    }

    if (curr_node->next != NULL) {
      curr_node->next->prev = new_node;
    }

    new_node->next = curr_node->next;
    curr_node      = temp;
  }
  while (curr_node->prev != NULL) {
    curr_node = curr_node->prev;
  }
  fpr->head = curr_node;

  return result;
}

enum libd_result
libd_filepath_resolver_normalize(struct filepath_resolver* fpr)
{
  // TODO:
  (void)fpr;
  return 0;
}

enum libd_result
libd_filepath_resolver_dump_to_filepath(
  struct filepath_resolver* fpr,
  struct filepath* dest)
{
  (void)fpr;
  (void)dest;
  // TODO:
  return 0;
}
