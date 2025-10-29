#include "../../../include/libdane/platform/filesystem.h"
#include "../../internal/internal.h"
#include "./parsing.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static enum libd_allocator_result
token_allocator_create(struct libd_token_allocator* out_ta);
static void
token_allocator_destroy(struct libd_token_allocator* ta);

enum libd_allocator_result
libd_path_tokenizer_create(
  struct libd_path_tokenizer** out_pt,
  const char* src_path,
  libd_platform_filesystem_path_type_e path_type)
{
  enum libd_allocator_result r = ok;

  struct libd_path_tokenizer* pt = malloc(sizeof(*pt));
  if (pt == NULL) {
    return enomem;
  }

  pt->src       = src_path;
  pt->path_type = path_type;

  r = token_allocator_create(&pt->token_allocator);
  if (r != ok) {
    free(pt);
    return enomem;
  }

  *out_pt = pt;

  return r;
}

void
libd_path_tokenizer_destroy(struct libd_path_tokenizer* pt)
{
  token_allocator_destroy(&pt->token_allocator);
  free(pt);
}

static token_type_e
path_tokenizer_make_token(struct libd_path_tokenizer* pt);

enum libd_allocator_result
libd_path_tokenizer_run(struct libd_path_tokenizer* pt)
{
  pt->token_allocator.reset(&pt->token_allocator.allocator);
  pt->scan_pos    = 0;
  pt->token_count = 0;

  token_type_e token_type;
  do {
    token_type = path_tokenizer_make_token(pt);
  } while (token_type != eof_type);

  return ok;
}

enum libd_allocator_result
libd_path_tokenizer_evaluate(
  struct libd_path_tokenizer* pt,
  char* dest)
{
  //
  return ok;
}

static enum libd_allocator_result
token_allocator_create(struct libd_token_allocator* out_ta)
{
  enum libd_allocator_result r =
    libd_allocator_wrapper_create(&out_ta->allocator, 8192, 1);
  if (r != ok) {
    return enomem;
  }

  out_ta->alloc = libd_allocator_wrapper_alloc;
  out_ta->reset = libd_allocator_wrapper_reset;

  return ok;
}

static void
token_allocator_destroy(struct libd_token_allocator* ta)
{
  libd_allocator_wrapper_destroy(&ta->allocator);
}

static token_type_e
path_tokenizer_make_token(struct libd_path_tokenizer* pt);
static token_type_e
path_tokenizer_make_separator_token(struct libd_path_tokenizer* pt);
static token_type_e
path_tokenizer_make_eof_token(struct libd_path_tokenizer* pt);
static token_type_e
path_tokenizer_make_component_token(struct libd_path_tokenizer* pt);

static token_type_e
path_tokenizer_make_token(struct libd_path_tokenizer* pt)
{
  switch (pt->src[pt->scan_pos]) {
  case '/':
    return path_tokenizer_make_separator_token(pt);
    break;
  case '\0':
    return path_tokenizer_make_eof_token(pt);
    break;
  default:
    return path_tokenizer_make_component_token(pt);
  }
}

static token_type_e
path_tokenizer_make_separator_token(struct libd_path_tokenizer* pt)
{
  struct libd_path_token* tok =
    (struct libd_path_token*)pt->token_allocator.alloc(
      &pt->token_allocator.allocator, 3);

  tok->type     = separator_type;
  tok->value[0] = '/';
  tok->value[1] = '\0';

  pt->token_stream[pt->token_count] = tok;

  pt->token_count += 1;
  pt->scan_pos += 1;

  return tok->type;
}

static token_type_e
path_tokenizer_make_eof_token(struct libd_path_tokenizer* pt)
{
  struct libd_path_token* tok =
    (struct libd_path_token*)pt->token_allocator.alloc(
      &pt->token_allocator.allocator, 2);

  tok->type     = eof_type;
  tok->value[0] = '\0';

  pt->token_stream[pt->token_count] = tok;

  pt->token_count += 1;

  return tok->type;
}

static token_type_e
path_tokenizer_make_component_token(struct libd_path_tokenizer* pt)
{
  size_t component_start = pt->scan_pos;
  while (pt->src[pt->scan_pos] != '/' && pt->src[pt->scan_pos] != '\0') {
    pt->scan_pos += 1;
  }
  size_t component_length = pt->scan_pos - component_start;

  struct libd_path_token* tok =
    (struct libd_path_token*)pt->token_allocator.alloc(
      &pt->token_allocator.allocator, sizeof(*tok) + component_length + 1);

  tok->type = component_type;
  memcpy(tok->value, &pt->src[component_start], component_length);
  tok->value[component_length] = '\0';

  pt->token_stream[pt->token_count] = tok;

  pt->token_count += 1;

  return tok->type;
}
