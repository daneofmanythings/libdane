#include "../../include/libd/memory.h"
#include "../../include/libd/testing.h"
#include "../../include/libd/utils/align_compat.h"

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>

static const size_t DEFAULT_CAPACITY = 128;

static libd_linear_allocator_h*
helper_create_linear_allocator(
  u32 reserve,
  u32 capacity,
  uint8_t alignment)
{
  libd_linear_allocator_h* allocator;
  ASSERT_OK(
    libd_linear_allocator_create(&allocator, reserve, capacity, alignment));

  return allocator;
}

TEST(linear_allocator_invalid_params)
{
  struct {
    char* name;
    u32 input_reserve;
    u32 input_capacity;
    u8 input_alignment;
    enum libd_result expected;
  } tcs[] = {
    {
      .name            = "valid\0",
      .input_reserve   = 4 * KiB,
      .input_capacity  = 16,
      .input_alignment = 2,
      .expected        = libd_ok,
    },
    {
      .name            = "zero capacity\0",
      .input_reserve   = 4 * KiB,
      .input_capacity  = 0,
      .input_alignment = 2,
      .expected        = libd_invalid_parameter,
    },
    {
      .name            = "zero alignment\0",
      .input_reserve   = 4 * KiB,
      .input_capacity  = 16,
      .input_alignment = 0,
      .expected        = libd_invalid_alignment,
    },
    {
      .name            = "non power of 2 alignment\0",
      .input_reserve   = 4 * KiB,
      .input_capacity  = 16,
      .input_alignment = 3,
      .expected        = libd_invalid_alignment,
    },
    {
      .name            = "unsupported alignment\0",
      .input_reserve   = 4 * KiB,
      .input_capacity  = 16,
      .input_alignment = LIBD_MAX_ALIGN * 2,
      .expected        = libd_invalid_alignment,
    },
  };

  // null 'out parameter'
  ASSERT_EQ_U(
    libd_linear_allocator_create(NULL, 4 * KiB, 16, 2), libd_invalid_parameter);

  size_t num_tests = ARR_LEN(tcs);
  for (size_t i = 0; i < num_tests; i += 1) {

    libd_linear_allocator_h* handle = NULL;

    ASSERT_EQ_U(
      libd_linear_allocator_create(
        &handle,
        tcs[i].input_reserve,
        tcs[i].input_capacity,
        tcs[i].input_alignment),
      tcs[i].expected);

    if (handle != NULL) {
      ASSERT_OK(libd_linear_allocator_destroy(handle));
    }
  }
}

TEST(linear_allocator_single_size)
{
  struct hex_hex {
    uint16_t top;
    uint16_t bottom;
  };

  const u32 reserve       = sysconf(_SC_PAGE_SIZE);
  const u32 cap           = 32;
  const size_t alloc_size = sizeof(struct hex_hex);
  const u8 align          = 2;
  const size_t num_loops  = reserve / alloc_size;

  libd_linear_allocator_h* la =
    helper_create_linear_allocator(cap, reserve, align);

  struct hex_hex hh[4 * KiB] = { 0 };
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

#define toks_len 8
TEST(linear_allocator_variable_size_alignment_one)
{
  struct token {
    uint8_t type;
    char value[];
  };

  const u32 reserve   = KiB;
  const u32 cap       = 32;
  const uint8_t align = 1;
  libd_linear_allocator_h* la =
    helper_create_linear_allocator(cap, reserve, align);

  // 39 non-null chars
  char* values[] = {
    "rectangle\0", "america\0", "megaphone\0", "monday\0", "butthole\0",
  };
  size_t vals_len = ARR_LEN(values);

  struct token* toks[toks_len] = { 0 };
  const size_t base_alloc_size = sizeof(struct token) + 1;

  size_t bytes_free = 0;

  size_t tok_count;
  size_t allocation_size;
  for (tok_count = 0; tok_count < toks_len && tok_count < vals_len;
       tok_count += 1) {
    ASSERT_OK(libd_linear_allocator_bytes_free(la, &bytes_free));
    size_t value_len = strlen(values[tok_count]);
    allocation_size  = base_alloc_size + value_len + 1;

    if (bytes_free < allocation_size) {
      break;
    }

    ASSERT_OK(libd_linear_allocator_alloc(
      la, (void**)&toks[tok_count], allocation_size));

    toks[tok_count]->type             = tok_count;
    toks[tok_count]->value[value_len] = '\0';
    memcpy(
      toks[tok_count]->value, values[tok_count], strlen(values[tok_count]));
  }

  for (size_t i = 0; i < tok_count; i += 1) {
    ASSERT_EQ_U(toks[i]->type, i);
    ASSERT_EQ_U(toks[i]->value[strlen(values[i])], '\0');
    ASSERT_EQ_STR(toks[i]->value, values[i]);
  }

  ASSERT_OK(libd_linear_allocator_destroy(la));
}
