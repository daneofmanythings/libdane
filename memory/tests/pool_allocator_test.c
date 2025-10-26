#include "../include/libdane/memory.h"

#include <criterion/criterion.h>
#include <criterion/internal/new_asserts.h>
#include <criterion/new/assert.h>
#include <criterion/parameterized.h>

libd_memory_pool_allocator_o*
helper_pool_allocator_create(
  uint32_t capacity,
  uint32_t size,
  uint8_t  alignment)
{
  libd_memory_pool_allocator_o* pool;
  libd_memory_result_e          result;

  result = libd_memory_pool_allocator_create(&pool, capacity, size, alignment);
  cr_assert(eq(u8, result, libd_mem_ok));

  return pool;
}

libd_memory_result_e
helper_pool_allocator_destroy(libd_memory_pool_allocator_o* pool)
{
  return libd_memory_pool_allocator_destroy(pool);
}

struct create_params {
  uint32_t             input_capacity;
  uint32_t             input_size;
  uint32_t             input_alignment;
  libd_memory_result_e expected;
};

ParameterizedTestParameters(
  pool_allocator,
  valid_parameters)
{
  struct create_params params[] = {
    {
      .input_capacity  = 0,
      .input_size      = 8,
      .input_alignment = 8,
      .expected        = libd_mem_invalid_zero_parameter,
    },
  };
  size_t nb_params = sizeof(params) / sizeof(struct create_params);
  return cr_make_param_array(struct create_params, params, nb_params, NULL);
}

ParameterizedTest(
  struct create_params* params,
  pool_allocator,
  valid_parameters)
{
  libd_memory_result_e result;

  libd_memory_pool_allocator_o* pool;
  result = libd_memory_pool_allocator_create(
    &pool, params->input_capacity, params->input_size, params->input_alignment);

  cr_assert(eq(u8, result, params->expected));
}
