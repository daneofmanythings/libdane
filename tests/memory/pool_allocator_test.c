#include "../../include/libd/memory.h"
#include "../../include/libd/testing.h"

#include <stdbool.h>

static libd_pool_allocator_h*
helper_pool_allocator_create(
  uint32_t capacity,
  uint32_t size,
  uint8_t alignment)
{
  libd_pool_allocator_h* pool;
  ASSERT_OK(libd_pool_allocator_create(&pool, capacity, size, alignment));

  return pool;
}

struct pool_allocator_create_param {
  uint32_t input_max_allocations;
  uint32_t input_bytes_per_allocation;
  uint32_t input_alignment;
  enum libd_result expected;
};

TEST(pool_allocator_invalid_params)
{
  static struct pool_allocator_create_param test_cases[] = {
    {
      .input_max_allocations      = 0,
      .input_bytes_per_allocation = 8,
      .input_alignment            = 8,
      .expected                   = libd_invalid_parameter,
    },
  };

  size_t num_tests = ARR_LEN(test_cases);
  for (size_t i = 0; i < num_tests; i += 1) {
    enum libd_result result;
    libd_pool_allocator_h* pool;

    result = libd_pool_allocator_create(
      &pool,
      test_cases[i].input_max_allocations,
      test_cases[i].input_bytes_per_allocation,
      test_cases[i].input_alignment);

    ASSERT_EQ_U(result, test_cases[i].expected);
  }
}
