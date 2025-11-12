#include "../../include/libd/memory.h"
#include "../../include/libd/testing.h"
#include "../../include/libd/utils/align_compat.h"

#include <stdbool.h>

static libd_pool_allocator_h*
helper_pool_allocator_create(
  u32 capacity,
  u32 size,
  u8 alignment)
{
  libd_pool_allocator_h* pool;
  ASSERT_OK(libd_pool_allocator_create(&pool, capacity, size, alignment));

  return pool;
}

TEST(pool_allocator_invalid_params)
{
  struct {
    const char* name;
    u32 input_max_allocations;
    u32 input_bytes_per_allocation;
    u32 input_alignment;
    enum libd_result expected;
  } tcs[] = {
    {
      .name                       = "zero max allocations\0",
      .input_max_allocations      = 0,
      .input_bytes_per_allocation = 8,
      .input_alignment            = 8,
      .expected                   = libd_invalid_parameter,
    },
    {
      .name                       = "zero bytes per allocation\0",
      .input_max_allocations      = 16,
      .input_bytes_per_allocation = 0,
      .input_alignment            = 8,
      .expected                   = libd_invalid_parameter,
    },
    {
      .name                       = "zero alignment\0",
      .input_max_allocations      = 16,
      .input_bytes_per_allocation = 8,
      .input_alignment            = 0,
      .expected                   = libd_invalid_alignment,
    },
    {
      .name                       = "non pow(2) alignment\0",
      .input_max_allocations      = 16,
      .input_bytes_per_allocation = 8,
      .input_alignment            = 3,
      .expected                   = libd_invalid_alignment,
    },
    {
      .name                       = "too big alignment\0",
      .input_max_allocations      = 16,
      .input_bytes_per_allocation = 8,
      .input_alignment            = LIBD_MAX_ALIGN * 2,
      .expected                   = libd_invalid_alignment,
    },
  };

  size_t num_tests = ARR_LEN(tcs);
  const char* name;
  for (size_t i = 0; i < num_tests; i += 1) {
    name = tcs[i].name;
    enum libd_result result;
    libd_pool_allocator_h* pa;

    result = libd_pool_allocator_create(
      &pa,
      tcs[i].input_max_allocations,
      tcs[i].input_bytes_per_allocation,
      tcs[i].input_alignment);

    ASSERT_EQ_U(result, tcs[i].expected, "name=%s\n", tcs[i].name);

    libd_pool_allocator_destroy(pa);
  }
}
