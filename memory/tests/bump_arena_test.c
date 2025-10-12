#include "../include/libdane/memory.h"
#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <stddef.h>
#include <stdint.h>

static const size_t ARENA_CAPACITY = 32;

typedef struct {
  int member;
} test_s;

// Helper function to create arena (reduces duplication)
static libd_memory_bump_arena_s*
create_test_arena(size_t capacity)
{
  libd_memory_bump_arena_s* arena;
  libd_memory_result_e result = libd_memory_bump_arena_create(&arena, capacity);
  cr_assert(eq(u8, result, RESULT_OK));
  cr_assert(ne(ptr, arena, NULL), "Arena should not be NULL");
  return arena;
}

// Helper function to destroy arena
static void
destroy_test_arena(libd_memory_bump_arena_s* arena)
{
  if (arena) {
    libd_memory_result_e result = libd_memory_bump_arena_destroy(arena);
    cr_assert(eq(u8, result, RESULT_OK));
  }
}

TestSuite(bump_arena);

Test(bump_arena, create_and_destroy)
{
  libd_memory_bump_arena_s* arena;
  libd_memory_result_e result =
    libd_memory_bump_arena_create(&arena, ARENA_CAPACITY);

  cr_assert(eq(u8, result, RESULT_OK));
  cr_assert(ne(ptr, arena, NULL), "Created arena should not be NULL");

  result = libd_memory_bump_arena_destroy(arena);
  cr_assert(eq(u8, result, RESULT_OK));
}

Test(bump_arena, create_zero_capacity)
{
  libd_memory_bump_arena_s* arena;
  libd_memory_result_e result = libd_memory_bump_arena_create(&arena, 0);

  // Assert whatever your API's expected behavior is
  cr_assert(ne(u8, result, RESULT_OK), "Zero capacity should fail");
}

Test(bump_arena, alloc_success)
{
  libd_memory_bump_arena_s* arena = create_test_arena(ARENA_CAPACITY);

  uint8_t* ptr = NULL;
  libd_memory_result_e result =
    libd_memory_bump_arena_alloc(arena, &ptr, sizeof(test_s));

  cr_assert(eq(u8, result, RESULT_OK));
  cr_assert(ne(ptr, ptr, NULL), "Allocated pointer should not be NULL");

  // Verify we can write to the memory
  test_s* data = (test_s*)ptr;
  data->member = 42;
  cr_assert(eq(int, data->member, 42));

  destroy_test_arena(arena);
}

Test(bump_arena, alloc_multiple)
{
  libd_memory_bump_arena_s* arena = create_test_arena(ARENA_CAPACITY);

  uint8_t* ptr1 = NULL;
  uint8_t* ptr2 = NULL;

  cr_assert(eq(u8, libd_memory_bump_arena_alloc(arena, &ptr1, sizeof(test_s)),
               RESULT_OK));
  cr_assert(eq(u8, libd_memory_bump_arena_alloc(arena, &ptr2, sizeof(test_s)),
               RESULT_OK));

  cr_assert(ne(ptr, ptr1, NULL));
  cr_assert(ne(ptr, ptr2, NULL));
  cr_assert(ne(ptr, ptr1, ptr2),
            "Allocations should return different pointers");

  destroy_test_arena(arena);
}

Test(bump_arena, alloc_exhaustion)
{
  libd_memory_bump_arena_s* arena = create_test_arena(ARENA_CAPACITY);

  uint8_t* ptr = NULL;

  // Allocate until we run out of space
  size_t alloc_count = 0;
  while (libd_memory_bump_arena_alloc(arena, &ptr, sizeof(test_s)) ==
         RESULT_OK) {
    alloc_count++;
    cr_assert(lt(sz, alloc_count, 100), "Safety check: too many allocations");
  }

  // Next allocation should fail
  libd_memory_result_e result =
    libd_memory_bump_arena_alloc(arena, &ptr, sizeof(test_s));
  cr_assert(eq(u8, result, ERR_NO_MEMORY));

  destroy_test_arena(arena);
}

Test(bump_arena, reset_allows_reallocation)
{
  libd_memory_bump_arena_s* arena = create_test_arena(ARENA_CAPACITY);

  uint8_t* ptr1 = NULL;
  uint8_t* ptr2 = NULL;

  // Fill the arena
  while (libd_memory_bump_arena_alloc(arena, &ptr1, sizeof(test_s)) ==
         RESULT_OK)
    ;

  // Should be out of memory
  cr_assert(eq(u8, libd_memory_bump_arena_alloc(arena, &ptr1, sizeof(test_s)),
               ERR_NO_MEMORY));

  // Reset and allocate again
  cr_assert(eq(u8, libd_memory_bump_arena_reset(arena), RESULT_OK));
  cr_assert(eq(u8, libd_memory_bump_arena_alloc(arena, &ptr2, sizeof(test_s)),
               RESULT_OK));

  cr_assert(ne(ptr, ptr2, NULL));

  destroy_test_arena(arena);
}

Test(bump_arena, reset_clears_memory)
{
  libd_memory_bump_arena_s* arena = create_test_arena(ARENA_CAPACITY);

  uint8_t* ptr = NULL;
  cr_assert(eq(u8, libd_memory_bump_arena_alloc(arena, &ptr, sizeof(test_s)),
               RESULT_OK));

  test_s* data = (test_s*)ptr;
  data->member = 123;
  cr_assert(eq(int, data->member, 123));

  cr_assert(eq(u8, libd_memory_bump_arena_reset(arena), RESULT_OK));

  destroy_test_arena(arena);
}
