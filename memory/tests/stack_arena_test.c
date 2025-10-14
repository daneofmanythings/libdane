#include "../include/libdane/memory.h"

#include <criterion/criterion.h>
#include <criterion/internal/new_asserts.h>
#include <criterion/new/assert.h>
#include <stddef.h>
#include <stdint.h>

static const size_t DEFAULT_CAP = 32;
static const size_t DEFAULT_STACK_CAP = 10;

struct small {
  uint8_t member;
};
struct medium {
  uint32_t member;
};
struct large {
  uint64_t member;
};

static libd_memory_stack_arena_s*
_stack_arena_create_helper(size_t cap, size_t stack_cap)
{
  libd_memory_stack_arena_s* p_arena;
  libd_memory_result_e result =
    libd_memory_stack_arena_create(&p_arena, cap, stack_cap);

  cr_assert(eq(u8, result, RESULT_OK));
  cr_assert(ne(ptr, p_arena, NULL));

  return p_arena;
}

static void
_stack_arena_destroy_helper(libd_memory_stack_arena_s* p_arena)
{
  cr_assert(ne(ptr, p_arena, NULL));
  libd_memory_result_e result = libd_memory_stack_arena_destroy(p_arena);
  cr_assert(eq(u8, result, RESULT_OK));
}

TestSuite(stack_arena);

Test(stack_arena, create_invalid_null_parameter)
{
  cr_assert(
    eq(u8, libd_memory_stack_arena_create(NULL, DEFAULT_CAP, DEFAULT_STACK_CAP),
       ERR_INVALID_NULL_PARAMETER));
}

Test(stack_arena, create_invalid_zero_value_parameter)
{
  libd_memory_stack_arena_s* p_arena;
  cr_assert(eq(u8,
               libd_memory_stack_arena_create(&p_arena, 0, DEFAULT_STACK_CAP),
               ERR_INVALID_ZERO_PARAMETER));
  cr_assert(eq(u8, libd_memory_stack_arena_create(&p_arena, DEFAULT_CAP, 0),
               ERR_INVALID_ZERO_PARAMETER));
}

Test(stack_arena, alloc_invalid_null_parameter)
{
  libd_memory_stack_arena_s* p_arena =
    _stack_arena_create_helper(DEFAULT_CAP, DEFAULT_STACK_CAP);

  uint8_t* p_data;
  cr_assert(eq(u8,
               libd_memory_stack_arena_alloc(p_arena, NULL, sizeof(uint8_t)),
               ERR_INVALID_NULL_PARAMETER));
  cr_assert(eq(u8,
               libd_memory_stack_arena_alloc(NULL, &p_data, sizeof(uint8_t)),
               ERR_INVALID_NULL_PARAMETER));

  _stack_arena_destroy_helper(p_arena);
}

Test(stack_arena, alloc_invalid_zero_value_parameter)
{
  libd_memory_stack_arena_s* p_arena =
    _stack_arena_create_helper(DEFAULT_CAP, DEFAULT_STACK_CAP);

  uint8_t* p_data;
  cr_assert(eq(u8, libd_memory_stack_arena_alloc(p_arena, &p_data, 0),
               ERR_INVALID_ZERO_PARAMETER));

  _stack_arena_destroy_helper(p_arena);
}

Test(stack_arena, alloc_stack_full)
{
  libd_memory_stack_arena_s* p_arena =
    _stack_arena_create_helper(DEFAULT_CAP, DEFAULT_STACK_CAP);

  uint8_t* handle;
  libd_memory_result_e result = RESULT_OK;
  while (result == RESULT_OK) {
    result =
      libd_memory_stack_arena_alloc(p_arena, &handle, sizeof(struct small));
  }
  cr_assert(eq(u8, result, ERR_STACK_FULL));

  _stack_arena_destroy_helper(p_arena);
}

Test(stack_arena, alloc_multiple_sizes)
{
  libd_memory_stack_arena_s* p_arena =
    _stack_arena_create_helper(DEFAULT_CAP, DEFAULT_STACK_CAP);

  uint8_t* handle;
  cr_assert(eq(
    u8, libd_memory_stack_arena_alloc(p_arena, &handle, sizeof(struct small)),
    RESULT_OK));
  cr_assert(eq(
    u8, libd_memory_stack_arena_alloc(p_arena, &handle, sizeof(struct medium)),
    RESULT_OK));
  cr_assert(eq(
    u8, libd_memory_stack_arena_alloc(p_arena, &handle, sizeof(struct large)),
    RESULT_OK));

  _stack_arena_destroy_helper(p_arena);
}

Test(stack_arena, reset_invalid_null_parameter)
{
  cr_assert(
    eq(u8, libd_memory_stack_arena_reset(NULL), ERR_INVALID_NULL_PARAMETER));
}

Test(stack_arena, reset_resets_all_memory)
{
  libd_memory_stack_arena_s* p_arena =
    _stack_arena_create_helper(DEFAULT_CAP, DEFAULT_STACK_CAP);

  size_t num_allocations = 0;
  uint8_t* handle;
  libd_memory_result_e result = RESULT_OK;
  while (result == RESULT_OK) {
    result =
      libd_memory_stack_arena_alloc(p_arena, &handle, sizeof(struct small));
    num_allocations += 1;
  }
  cr_assert(eq(u8, result, ERR_STACK_FULL));

  cr_assert(eq(u8, libd_memory_stack_arena_reset(p_arena), RESULT_OK));

  result = RESULT_OK;
  while (result == RESULT_OK) {
    result =
      libd_memory_stack_arena_alloc(p_arena, &handle, sizeof(struct small));
    num_allocations -= 1;
  }
  cr_assert(eq(u8, result, ERR_STACK_FULL));
  cr_assert(eq(u8, num_allocations, 0));

  _stack_arena_destroy_helper(p_arena);
}

Test(stack_arena, free_invalid_null_parameter)
{
  cr_assert(
    eq(u8, libd_memory_stack_arena_free(NULL), ERR_INVALID_NULL_PARAMETER));
}

Test(stack_arena, free_stack_empty)
{
  libd_memory_stack_arena_s* p_arena =
    _stack_arena_create_helper(DEFAULT_CAP, DEFAULT_STACK_CAP);

  cr_assert(eq(u8, libd_memory_stack_arena_free(p_arena), ERR_STACK_EMPTY));

  _stack_arena_destroy_helper(p_arena);
}
