#include "../include/libdane/memory.h"

#include <criterion/criterion.h>
#include <criterion/new/assert.h>
#include <criterion/parameterized.h>

libd_memory_pool_allocator_ot*
helper_pool_allocator_create(
  uint32_t capacity,
  uint32_t size,
  uint8_t alignment)
{
  libd_memory_pool_allocator_ot* pool;
  libd_memory_result_e result;

  result = libd_memory_pool_allocator_create(&pool, capacity, size, alignment);
  cr_assert(eq(u8, result, libd_mem_ok));

  return pool;
}

libd_memory_result_e
helper_pool_allocator_destroy(libd_memory_pool_allocator_ot* pool)
{
  return libd_memory_pool_allocator_destroy(pool);
}

struct create_param {
  uint32_t input_max_allocations;
  uint32_t input_bytes_per_allocation;
  uint32_t input_alignment;
  libd_memory_result_e expected;
};

ParameterizedTestParameters(
  pool_allocator,
  invalid_parameters)
{
  static struct create_param params[] = {
    {
      .input_max_allocations      = 0,
      .input_bytes_per_allocation = 8,
      .input_alignment            = 8,
      .expected                   = libd_mem_invalid_zero_parameter,
    },
  };
  size_t nb_params = sizeof(params) / sizeof(struct create_param);
  return cr_make_param_array(struct create_param, params, nb_params, NULL);
}

ParameterizedTest(
  struct create_param* param,
  pool_allocator,
  invalid_parameters)
{
  libd_memory_result_e result;

  libd_memory_pool_allocator_ot* pool;
  result = libd_memory_pool_allocator_create(
    &pool,
    param->input_max_allocations,
    param->input_bytes_per_allocation,
    param->input_alignment);

  cr_assert(eq(u8, result, param->expected));
}
