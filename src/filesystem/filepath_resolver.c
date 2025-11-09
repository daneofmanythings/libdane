#include "../../include/libd/filesystem.h"
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
libd_tokenize_from_string_into_dll(
  const char* src,
  struct filepath_allocator* fpa);

enum libd_result
libd_filepath_resolver_tokenize(struct filepath_resolver* fpr)
{
  fpr->allocator.reset(&fpr->allocator.wrapper);
  fpr->head = libd_tokenize_from_string_into_dll(fpr->src, &fpr->allocator);

  return libd_ok;
}

static struct path_token_node*
libd_make_path_token_node(
  const char* src,
  size_t scan_pos,
  struct filepath_allocator* fpa);
static struct path_token_node*
libd_make_separator_token(struct filepath_allocator* fpa);
static struct path_token_node*
libd_make_eof_token(struct filepath_allocator* fpa);
static struct path_token_node*
libd_make_component_token(
  const char* src,
  size_t scan_pos,
  struct filepath_allocator* fpa);

struct path_token_node*
libd_tokenize_from_string_into_dll( // FIX: this needs error handling
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

  } while (!IS_EOF_TYPE(node->type));

  return head;
}

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
  memcpy(node->value, &src[component_start], component_length);
  node->value[component_length] = NULL_TERMINATOR;
  node->val_len                 = component_length;

  if (strcmp(node->value, "..\0") == 0) {
    node->type = parent_ref_type;
  } else if (strcmp(node->value, ".\0") == 0) {
    node->type = self_ref_type;
  } else {
    node->type = segment_type;
  }

  return node;
}

enum libd_result
libd_filepath_resolver_expand( // FIX: error handling
  struct filepath_resolver* fpr,
  libd_filesystem_env_get_f env_get)
{
  struct path_token_node* curr_node   = fpr->head;
  struct path_token_node* temp        = NULL;
  struct path_token_node* result_head = NULL;

  char env_val[256] = { 0 };

  enum libd_result result = libd_ok;

  while (!IS_EOF_TYPE(curr_node->type)) {
    bool is_env_var = platform_is_env_var(curr_node->value);

    if (!is_env_var) {
      if (result_head == NULL) {
        result_head = curr_node;
      }
      curr_node = curr_node->next;
      continue;
    }

    result = platform_env_var_get(env_val, curr_node->value, env_get);
    if (result != libd_ok) {
      return result;
    }

    struct path_token_node* new_node =
      libd_tokenize_from_string_into_dll(env_val, &fpr->allocator);

    if (curr_node->prev) {
      curr_node->prev->next = new_node;
    }
    new_node->prev = curr_node->prev;

    temp = new_node;
    if (new_node->next != NULL) {
      while (!IS_EOF_TYPE(new_node->next->type)) {
        new_node = new_node->next;
      }
    }

    if (curr_node->next != NULL) {
      curr_node->next->prev = new_node;
    }

    new_node->next = curr_node->next;
    curr_node      = temp;
  }
  fpr->head = result_head;

  return result;
}

enum libd_result
handle_path_separator(struct path_token_node* node);
enum libd_result
handle_filepath_parent_ref(
  struct path_token_node* node,
  struct filepath_resolver* fpr);
enum libd_result
handle_filepath_curr_ref(
  struct path_token_node* node,
  struct filepath_resolver* fpr);
enum libd_result
handle_filepath_component(struct path_token_node* node);
void
handle_leading_and_trailing_separators(struct filepath_resolver* fpr);

enum libd_result
libd_filepath_resolver_normalize(struct filepath_resolver* fpr)
{
  enum libd_result r = libd_ok;

  struct path_token_node* curr_node = fpr->head;
  while (1) {
    switch (curr_node->type) {
    case eof_type:
      goto done;
    case separator_type:
      r = handle_path_separator(curr_node);
      break;
    case parent_ref_type:
      r = handle_filepath_parent_ref(curr_node, fpr);
      break;
    case self_ref_type:
      r = handle_filepath_curr_ref(curr_node, fpr);
      break;
    default:
      r = handle_filepath_component(curr_node);
    }
    if (r != libd_ok) {
      return r;
    }
    curr_node = curr_node->next;
  }

done:
  handle_leading_and_trailing_separators(fpr);

  return libd_ok;
}

enum libd_result
handle_path_separator(struct path_token_node* node)
{
  if (node->prev != NULL && IS_SEPARATOR_TYPE(node->prev->type)) {
    node->prev->next = node->next;
    node->next->prev = node->prev;
  }

  return libd_ok;
}

enum libd_result
handle_filepath_parent_ref(
  struct path_token_node* node,
  struct filepath_resolver* fpr)
{
  if (node->next == NULL || !IS_SEPARATOR_TYPE(node->next->type)) {
    return libd_invalid_path;
  }

  if (node->prev == NULL) {
    if (libd_filepath_is_abs(fpr->path_type)) {
      return libd_invalid_path;
    }
    return libd_ok;
  }

  struct path_token_node* one_behind = node->prev;
  if (!IS_SEPARATOR_TYPE(one_behind->type)) {
    return libd_invalid_path;
  }

  if (libd_filepath_is_abs(fpr->path_type)) {
    return libd_invalid_path;
  }

  struct path_token_node* two_behind = one_behind->prev;

  if (two_behind == NULL) {
    if (libd_filepath_is_abs(fpr->path_type)) {
      return libd_invalid_path;
    }
    fpr->head = node;
    return libd_ok;
  }

  if (!IS_COMPONENT_TYPE(two_behind->type)) {
    return libd_invalid_path;
  }

  if (IS_PARENT_REF_TYPE(two_behind->type)) {
    return libd_ok;  // chain of relative parent refs
  }

  node->next->prev = two_behind->prev;
  if (two_behind->prev != NULL) {
    two_behind->prev->next = node->next;
  } else {
    fpr->head = node->next;
  }

  return libd_ok;
}

enum libd_result
handle_filepath_curr_ref(
  struct path_token_node* node,
  struct filepath_resolver* fpr)
{
  if (node->next == NULL || IS_EOF_TYPE(node->next->type)) {
    return libd_invalid_path;
  }

  node->next->prev = node->prev;
  if (node->prev != NULL) {
    node->prev->next = node->next;
  } else {
    fpr->head = node->next;
  }

  return libd_ok;
}

enum libd_result
handle_filepath_component(struct path_token_node* node)
{
  if (!plaform_is_component_value_valid(node->value)) {
    return libd_invalid_path;
  }
  if (node->prev != NULL && !IS_SEPARATOR_TYPE(node->prev->type)) {
    return libd_invalid_path;
  }
  if ( // purposely not doing compliment logic for type.
    node->next == NULL || 
    !(IS_EOF_TYPE(node->next->type) || IS_SEPARATOR_TYPE(node->next->type))) {
    return libd_invalid_path;
  }

  return libd_ok;
}

static inline void
ensure_no_leading_separator(struct filepath_resolver* fpr);
static inline void
ensure_no_trailing_separator(struct filepath_resolver* fpr);
static inline void
ensure_leading_separator(struct filepath_resolver* fpr);
static inline void
ensure_trailing_separator(struct filepath_resolver* fpr);

void
handle_leading_and_trailing_separators(struct filepath_resolver* fpr)
{
  switch (fpr->path_type) {
  case libd_rel_file:
    ensure_no_leading_separator(fpr);
    ensure_no_trailing_separator(fpr);
    break;
  case libd_abs_file:
    ensure_leading_separator(fpr);
    ensure_no_trailing_separator(fpr);
    break;
  case libd_rel_directory:
    ensure_no_leading_separator(fpr);
    ensure_trailing_separator(fpr);
    break;
  case libd_abs_directory:
    ensure_leading_separator(fpr);
    ensure_trailing_separator(fpr);
    break;
  }
}

static inline void
ensure_no_leading_separator(struct filepath_resolver* fpr)
{
  if (IS_SEPARATOR_TYPE(fpr->head->type)) {
    fpr->head->next->prev = fpr->head->prev;
    // fpr->head->prev->next = fpr->head->next;
    fpr->head = fpr->head->next;
  }
}
static inline void
ensure_no_trailing_separator(struct filepath_resolver* fpr)
{
  struct path_token_node* curr = fpr->head;
  while (!IS_EOF_TYPE(curr->next->type)) {
    curr = curr->next;
  }

  if (IS_PARENT_REF_TYPE(curr->prev->type)) {
    return;
  }

  if (IS_SEPARATOR_TYPE(curr->type)) {
    curr->prev->next = curr->next;
    curr->next->prev = curr->prev;
  }
}
static inline void
ensure_leading_separator(struct filepath_resolver* fpr)
{
  if (IS_SEPARATOR_TYPE(fpr->head->type)) {
    return;
  }

  struct path_token_node* new_head = libd_make_separator_token(&fpr->allocator);

  new_head->prev  = fpr->head->prev;
  new_head->next  = fpr->head;
  fpr->head->prev = new_head;
  // new_head->prev->next = new_head;

  fpr->head = new_head;
}
static inline void
ensure_trailing_separator(struct filepath_resolver* fpr)
{
  struct path_token_node* curr = fpr->head;
  while (!IS_EOF_TYPE(curr->next->type)) {
    curr = curr->next;
  }

  if (IS_SEPARATOR_TYPE(curr->type)) {
    return;
  }

  struct path_token_node* new_tail = libd_make_separator_token(&fpr->allocator);

  new_tail->prev   = curr;
  new_tail->next   = curr->next;
  curr->next->prev = new_tail;
  curr->next       = new_tail;
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
