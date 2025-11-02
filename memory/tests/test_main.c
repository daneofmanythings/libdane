#include "../../testing/include/libdane/testing.h"
#include "./linear_allocator_test.c"
#include "./pool_allocator_test.c"

TEST_MAIN

// pool allocator
REGISTER(pool_allocator_invalid_params);

// linear allocator
REGISTER(linear_allocator_invalid_params);
REGISTER(linear_allocator_single_size);
REGISTER(linear_allocator_variable_size_alignment_one);

END_TEST_MAIN
