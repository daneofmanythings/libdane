#include "../../testing/include/libdane/testing.h"
#include "../include/libdane/internal/align_compat.h"
#include "../include/libdane/memory.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>

static const size_t DEFAULT_CAPACITY = 128;

struct create_inputs {
  size_t capacity;
  uint8_t alignment;
};

libd_linear_allocator_h*
helper_create_linear_allocator(
  size_t capacity,
  uint8_t alignment)
{
  libd_linear_allocator_h* allocator;
  ASSERT_OK(libd_linear_allocator_create(&allocator, capacity, alignment));

  return allocator;
}

struct linear_allocator_create_param {
  char* name;
  struct create_inputs inputs;
  enum libd_result expected;
};

TEST(linear_allocator_invalid_params)
{
  struct linear_allocator_create_param test_cases[] = {
    {
      .name = "valid\0",
      .inputs = {
        16, 2,
      },
      .expected = libd_ok,
    },
    {
      .name = "zero capacity\0",
      .inputs = {
        0, 2,
      },
      .expected = libd_invalid_parameter,
    },
    {
      .name = "zero alignment\0",
      .inputs = {
        16, 0,
      },
      .expected = libd_invalid_alignment,
    },
    {
      .name = "non power of 2 alignment\0",
      .inputs = {
        16, 3,
      },
      .expected = libd_invalid_alignment,
    },
    {
      .name = "non power of 2 alignment\0",
      .inputs = {
        16, 2 * LIBD_MAX_ALIGN,
      },
      .expected = libd_invalid_alignment,
    },
  };

  // null 'out parameter'
  ASSERT_EQ_U(
    libd_linear_allocator_create(NULL, 16, 2), libd_invalid_parameter);

  size_t num_tests = ARR_LEN(test_cases);
  for (size_t i = 0; i < num_tests; i += 1) {

    libd_linear_allocator_h* handle = NULL;

    ASSERT_EQ_U(
      libd_linear_allocator_create(
        &handle, test_cases[i].inputs.capacity, test_cases[i].inputs.alignment),
      test_cases[i].expected);

    if (handle != NULL) {
      ASSERT_OK(libd_linear_allocator_destroy(handle));
    }
  }
}

struct alloc_param {
  char* name;
  struct create_inputs inputs;
  enum libd_result expected;
};

TEST(linear_allocator_single_size)
{
  struct hex_hex {
    uint16_t top;
    uint16_t bottom;
  };

  const size_t cap        = 32;
  const size_t alloc_size = sizeof(struct hex_hex);
  const uint8_t align     = 2;
  const size_t num_loops  = cap / alloc_size;

  libd_linear_allocator_h* la = helper_create_linear_allocator(cap, align);

  struct hex_hex hh[64] = { 0 };
  for (size_t i = 0; i < num_loops; i++) {
    ASSERT_OK(libd_linear_allocator_alloc(la, (void**)&hh[i], alloc_size));
    hh[i].top    = i;
    hh[i].bottom = i;
  }

  ASSERT_EQ_U(
    libd_linear_allocator_alloc(la, (void**)&hh[9], alloc_size),
    libd_no_memory);

  for (size_t i = 0; i < num_loops; i++) {
    ASSERT_EQ_U(hh[i].top, i);
    ASSERT_EQ_U(hh[i].bottom, i);
  }
}

TEST(linear_allocator_variable_size_alignment_one)
{
  struct token {
    uint8_t type;
    char value[];
  };

  const size_t cap             = 32;
  const size_t base_alloc_size = sizeof(struct token) + 1;
  const uint8_t align          = 1;

  libd_linear_allocator_h* la = helper_create_linear_allocator(cap, align);

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
    ASSERT_OK(libd_linear_allocator_bytes_free(la, &bytes_free));
    allocation_size = base_alloc_size + strlen(values[index]) + 1;

    if (bytes_free < allocation_size) {
      break;
    }

    ASSERT_OK(
      libd_linear_allocator_alloc(la, (void**)&toks[index], allocation_size));

    toks[index]->type         = index;
    toks[index]->value[index] = '\0';
    memcpy(toks[index]->value, values[index], strlen(values[index]));
  }

  ASSERT_EQ_U(
    libd_linear_allocator_alloc(
      la, (void**)&toks[index], base_alloc_size + index),
    libd_no_memory);

  for (size_t i = 0; i < index; i += 1) {
    ASSERT_EQ_U(toks[i]->type, i);
    ASSERT_EQ_U(toks[i]->value[strlen(values[i])], '\0');
    ASSERT_EQ_STR(toks[i]->value, values[i]);
  }

  ASSERT_OK(libd_linear_allocator_destroy(la));
}
