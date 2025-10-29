#include "../include/libdane/internal/align_compat.h"
#include "../include/libdane/memory.h"

#include <criterion/criterion.h>
#include <criterion/logging.h>
#include <criterion/new/assert.h>
#include <criterion/parameterized.h>
#include <libdane/memory.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const size_t DEFAULT_CAPACITY = 128;

struct create_inputs {
  size_t capacity;
  uint8_t alignment;
};

libd_memory_linear_allocator_ot*
helper_create_linear_allocator(
  size_t capacity,
  uint8_t alignment)
{
  libd_memory_linear_allocator_ot* allocator;
  libd_memory_result_e result =
    libd_memory_linear_allocator_create(&allocator, capacity, alignment);
  cr_assert(eq(u8, result, libd_mem_ok));

  return allocator;
}

void
helper_destroy_linear_allocator(libd_memory_linear_allocator_ot* p_allocator)
{
  libd_memory_result_e result =
    libd_memory_linear_allocator_destroy(p_allocator);
  cr_assert(eq(u8, result, libd_mem_ok));
}

struct create_param {
  char* name;
  struct create_inputs inputs;
  libd_memory_result_e expected;
};

ParameterizedTestParameters(
  linear_allocator,
  create_invalid_parameters)
{
  static struct create_param params[] = {
    {
      .name = "valid\0",
      .inputs = {
        16, 2,
      },
      .expected = libd_mem_ok,
    },
    {
      .name = "zero capacity\0",
      .inputs = {
        0, 2,
      },
      .expected = libd_mem_invalid_zero_parameter,
    },
    {
      .name = "zero alignment\0",
      .inputs = {
        16, 0,
      },
      .expected = libd_mem_invalid_alignment,
    },
    {
      .name = "non power of 2 alignment\0",
      .inputs = {
        16, 3,
      },
      .expected = libd_mem_invalid_alignment,
    },
    {
      .name = "non power of 2 alignment\0",
      .inputs = {
        16, 2 * LIBD_MAX_ALIGN,
      },
      .expected = libd_mem_invalid_alignment,
    },
  };

  size_t nb_params = sizeof(params) / sizeof(params[0]);
  return cr_make_param_array(struct create_param, params, nb_params, NULL);
}

ParameterizedTest(
  struct create_param* param,
  linear_allocator,
  create_invalid_parameters)
{
  // null 'out parameter'
  cr_assert(eq(
    u8,
    libd_memory_linear_allocator_create(NULL, 16, 2),
    libd_mem_invalid_null_parameter));

  libd_memory_linear_allocator_ot* handle = NULL;

  cr_assert(
    eq(
      u8,
      libd_memory_linear_allocator_create(
        &handle, param->inputs.capacity, param->inputs.alignment),
      param->expected),
    "%s",
    param->name);

  if (handle != NULL) {
    helper_destroy_linear_allocator(handle);
  }
}

struct alloc_param {
  char* name;
  struct create_inputs inputs;
  libd_memory_result_e expected;
};

Test(
  linear_allocator,
  alloc_single_size)
{
  struct hex_hex {
    uint16_t top;
    uint16_t bottom;
  };

  const size_t cap        = 32;
  const size_t alloc_size = sizeof(struct hex_hex);
  const uint8_t align     = 2;
  const size_t num_loops  = cap / alloc_size;

  libd_memory_linear_allocator_ot* la =
    helper_create_linear_allocator(cap, align);

  struct hex_hex hh[64] = { 0 };
  for (size_t i = 0; i < num_loops; i++) {
    cr_assert(eq(
      u8,
      libd_memory_linear_allocator_alloc(la, (void**)&hh[i], alloc_size),
      libd_mem_ok));
    hh[i].top    = i;
    hh[i].bottom = i;
  }

  cr_assert(eq(
    u8,
    libd_memory_linear_allocator_alloc(la, (void**)&hh[9], alloc_size),
    libd_mem_no_memory));

  for (size_t i = 0; i < num_loops; i++) {
    cr_assert(eq(u16, hh[i].top, i));
    cr_assert(eq(u16, hh[i].bottom, i));
  }
}

Test(
  linear_allocator,
  alloc_variable_size_align_1)
{
  struct token {
    uint8_t type;
    char value[];
  };

  const size_t cap             = 32;
  const size_t base_alloc_size = sizeof(struct token) + 1;
  const uint8_t align          = 1;

  libd_memory_linear_allocator_ot* la =
    helper_create_linear_allocator(cap, align);

  char* values[] = {
    "america\0",
    "flag\0",
    "rectangle\0",
    "butthole\0",
  };

  struct token* toks[64] = { 0 };

  size_t bytes_free = 0;

  size_t index;
  size_t allocation_size;
  for (index = 0; index < 64; index += 1) {
    cr_assert(eq(
      u8,
      libd_memory_linear_allocator_bytes_free(la, &bytes_free),
      libd_mem_ok));
    allocation_size = base_alloc_size + strlen(values[index]) + 1;

    if (bytes_free < allocation_size) {
      break;
    }

    cr_assert(eq(
      u8,
      libd_memory_linear_allocator_alloc(
        la, (void**)&toks[index], allocation_size),
      libd_mem_ok));

    toks[index]->type         = index;
    toks[index]->value[index] = '\0';
    memcpy(toks[index]->value, values[index], strlen(values[index]));
  }

  cr_assert(eq(
    u8,
    libd_memory_linear_allocator_alloc(
      la, (void**)&toks[index], base_alloc_size + index),
    libd_mem_no_memory));

  for (size_t i = 0; i < index; i += 1) {
    cr_assert(eq(u8, toks[i]->type, i));
    cr_assert(eq(chr, toks[i]->value[strlen(values[i])], '\0'));
    cr_assert(eq(str, toks[i]->value, values[i]));
  }

  helper_destroy_linear_allocator(la);
}
