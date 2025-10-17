#include "../include/libdane/internal/align_compat.h"
#include "../include/libdane/memory.h"

#include <criterion/criterion.h>
#include <criterion/internal/new_asserts.h>
#include <criterion/new/assert.h>
#include <stddef.h>
#include <stdint.h>

static const size_t DEFAULT_CAPACITY = 128;

typedef struct {
  uint8_t member;
} little_s;

typedef struct {
  uint16_t member;
} medium_s;

typedef struct {
  uint32_t member;
} large_s;

libd_memory_linear_allocator_s*
helper_create_linear_allocator(size_t capacity, uint8_t alignment)
{
  libd_memory_linear_allocator_s* allocator;
  libd_memory_result_e result =
    libd_memory_linear_allocator_create(&allocator, capacity, alignment);
  cr_assert(eq(u8, result, RESULT_OK));

  return allocator;
}

void
helper_destroy_linear_allocator(libd_memory_linear_allocator_s* p_allocator)
{
  libd_memory_result_e result =
    libd_memory_linear_allocator_destroy(p_allocator);
  cr_assert(eq(u8, result, RESULT_OK));
}

Test(linear_allocator, create_invalid_parameters)
{
  // null out parameter
  cr_assert(eq(u8, libd_memory_linear_allocator_create(NULL, 16, 2),
               ERR_INVALID_NULL_PARAMETER));

  libd_memory_linear_allocator_s* handle;

  // zero capacity
  cr_assert(eq(u8, libd_memory_linear_allocator_create(&handle, 0, 2),
               ERR_INVALID_ZERO_PARAMETER));

  // invalid alignment values
  cr_assert(eq(u8, libd_memory_linear_allocator_create(&handle, 16, 0),
               ERR_INVALID_ZERO_PARAMETER));
  cr_assert(eq(u8, libd_memory_linear_allocator_create(&handle, 16, 3),
               ERR_INVALID_ALIGNMENT));
  cr_assert(
    eq(u8, libd_memory_linear_allocator_create(&handle, 16, LIBD_MAX_ALIGN * 2),
       ERR_INVALID_ALIGNMENT));
}

Test(linear_allocator, alloc_general_use)
{
  libd_memory_linear_allocator_s* la =
    helper_create_linear_allocator(DEFAULT_CAPACITY, LIBD_ALIGNOF(medium_s));

  // TODO: finish

  helper_destroy_linear_allocator(la);
}

Test(linear_allocator, alloc_invalid_parameters)
{
  libd_memory_linear_allocator_s* la =
    helper_create_linear_allocator(DEFAULT_CAPACITY, LIBD_ALIGNOF(medium_s));

  void* op;
  cr_assert(eq(u8,
               libd_memory_linear_allocator_alloc(la, NULL, sizeof(medium_s)),
               ERR_INVALID_NULL_PARAMETER));
  cr_assert(eq(u8,
               libd_memory_linear_allocator_alloc(NULL, &op, sizeof(medium_s)),
               ERR_INVALID_NULL_PARAMETER));
  cr_assert(eq(u8,
               libd_memory_linear_allocator_alloc(NULL, &op, sizeof(medium_s)),
               ERR_INVALID_NULL_PARAMETER));


  helper_destroy_linear_allocator(la);
}

// TODO: FINISH THE TEST SUITE
