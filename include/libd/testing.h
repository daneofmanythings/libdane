/**
 * @file libd_testing.h
 * @brief Testing for libdane.
 */

#ifndef LIBD_TESTING_H
#define LIBD_TESTING_H

#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _LIBD_TYPES
  #include <stdint.h>
#endif

#ifndef LIBD_TESTING_ABORT_ON_FAIL
  #define LIBD_TESTING_ABORT_ON_FAIL 1
#endif

enum libd_test_code {
  test_ok,
  test_fail,
  test_skipped,
  //
  test_result_count,
};

/**
 * @brief function type for the testing functions
 */
typedef void (*test_function_f)(void);

struct libd_test_result {
  enum libd_test_code code;
  char message[128];
};

/**
 * @brief Structure holding test metadata
 */
struct libd_test_entry {
  const char* name;
  test_function_f f;
  struct libd_test_entry* next;
  // struct libd_test_result result; // may want later for better reports.
};

/**
 * @brief Global registry of tests to run
 */
static struct libd_test_entry* g_test_registry = NULL;

static void
register_test(struct libd_test_entry* entry)
{
  entry->next     = g_test_registry;
  g_test_registry = entry;
}

#define _LIBD_REGISTER_TEST(test_name)                 \
  static void _func_##test_name(void);                 \
  static struct libd_test_entry _entry_##test_name = { \
    .name = #test_name,                                \
    .f    = _func_##test_name,                         \
    .next = NULL,                                      \
  };                                                   \
  static void _register_##test_name(void)              \
  {                                                    \
    register_test(&_entry_##test_name);                \
  }                                                    \
  static void _func_##test_name(void)

#define TEST(name) _LIBD_REGISTER_TEST(name)

#define REGISTER(test_name) _register_##test_name()

#define TEST_MAIN                  \
  int main(int argc, char* argv[]) \
  {                                \
    (void)argc;                    \
    (void)argv;

#define END_TEST_MAIN                        \
  while (g_test_registry != NULL) {          \
    g_test_registry->f();                    \
    g_test_registry = g_test_registry->next; \
  }                                          \
  return 0;                                  \
  }

#define ASSERT_OP(lhs, op, rhs, type, repr)                             \
  do {                                                                  \
    type _lhs = (type)(lhs);                                            \
    type _rhs = (type)(rhs);                                            \
    if (!(_lhs op _rhs)) {                                              \
      fprintf(                                                          \
        stderr,                                                         \
        "Assertion failed: %s %s %s (" repr " vs " repr ") at %s:%d\n", \
        #lhs,                                                           \
        #op,                                                            \
        #rhs,                                                           \
        _lhs,                                                           \
        _rhs,                                                           \
        __FILE__,                                                       \
        __LINE__);                                                      \
      if (LIBD_TESTING_ABORT_ON_FAIL) {                                 \
        abort();                                                        \
      }                                                                 \
    }                                                                   \
  } while (0)

#define ASSERT_EQ(lhs, rhs, type, repr) ASSERT_OP(lhs, ==, rhs, type, repr)
#define ASSERT_NE(lhs, rhs, type, repr) ASSERT_OP(lhs, !=, rhs, type, repr)
#define ASSERT_LT(lhs, rhs, type, repr) ASSERT_OP(lhs, <, rhs, type, repr)
#define ASSERT_LE(lhs, rhs, type, repr) ASSERT_OP(lhs, <=, rhs, type, repr)
#define ASSERT_GT(lhs, rhs, type, repr) ASSERT_OP(lhs, >, rhs, type, repr)
#define ASSERT_GE(lhs, rhs, type, repr) ASSERT_OP(lhs, >=, rhs, type, repr)

#define ASSERT_TRUE(expr)  ASSERT_EQ(expr, 1, int, "%d")
#define ASSERT_FALSE(expr) ASSERT_EQ(expr, 0, int, "%d")

#define ASSERT_ZERO(expr)    ASSERT_FALSE(expr)
#define ASSERT_NONZERO(expr) ASSERT_NE(expr, 0, int, "%d")

#define ASSERT_OK(expr) ASSERT_EQ(expr, 0, int, "%d")

#define ASSERT_EQ_U(lhs, rhs) ASSERT_EQ(lhs, rhs, uint64_t, "%" PRIu64)
#define ASSERT_NE_U(lhs, rhs) ASSERT_NE(lhs, rhs, uint64_t, "%" PRIu64)
#define ASSERT_LT_U(lhs, rhs) ASSERT_LT(lhs, rhs, uint64_t, "%" PRIu64)
#define ASSERT_LE_U(lhs, rhs) ASSERT_LE(lhs, rhs, uint64_t, "%" PRIu64)
#define ASSERT_GT_U(lhs, rhs) ASSERT_GT(lhs, rhs, uint64_t, "%" PRIu64)
#define ASSERT_GE_U(lhs, rhs) ASSERT_GE(lhs, rhs, uint64_t, "%" PRIu64)

#define ASSERT_EQ_S(lhs, rhs) ASSERT_EQ(lhs, rhs, int64_t, "%" PRId64)
#define ASSERT_NE_S(lhs, rhs) ASSERT_NE(lhs, rhs, int64_t, "%" PRId64)
#define ASSERT_LT_S(lhs, rhs) ASSERT_LT(lhs, rhs, int64_t, "%" PRId64)
#define ASSERT_LE_S(lhs, rhs) ASSERT_LE(lhs, rhs, int64_t, "%" PRId64)
#define ASSERT_GT_S(lhs, rhs) ASSERT_GT(lhs, rhs, int64_t, "%" PRId64)
#define ASSERT_GE_S(lhs, rhs) ASSERT_GE(lhs, rhs, int64_t, "%" PRId64)

#define ASSERT_EQ_PTR(lhs, rhs) ASSERT_EQ(lhs, rhs, void*, "%p")
#define ASSERT_NE_PTR(lhs, rhs) ASSERT_NE(lhs, rhs, void*, "%p")
#define ASSERT_NULL(ptr)        ASSERT_EQ_PTR(ptr, NULL)
#define ASSERT_NOT_NULL(ptr)    ASSERT_NE_PTR(ptr, NULL)

#define ASSERT_EQ_MEM(lhs, rhs, len)                              \
  do {                                                            \
    const void* _lhs = (void*)(lhs);                              \
    const void* _rhs = (void*)(rhs);                              \
    size_t _len      = (len);                                     \
    if ((memcmp(_lhs, _rhs, _len)) != 0) {                        \
      fprintf(                                                    \
        stderr,                                                   \
        "Assertion failed: (%s) != (%s) ( %p vs %p ) at %s:%d\n", \
        #lhs,                                                     \
        #rhs,                                                     \
        _lhs,                                                     \
        _rhs,                                                     \
        __FILE__,                                                 \
        __LINE__);                                                \
      if (LIBD_TESTING_ABORT_ON_FAIL) {                           \
        abort();                                                  \
      }                                                           \
    }                                                             \
  } while (0)

/*
 * maybe add mem: NE, zero'd
 */

#define ASSERT_EQ_STR(lhs, rhs)                               \
  do {                                                        \
    const char* _lhs = (lhs);                                 \
    const char* _rhs = (rhs);                                 \
    if ((strcmp(_lhs, _rhs)) != 0) {                          \
      fprintf(                                                \
        stderr,                                               \
        "Assertion failed: %s != %s ( %s vs %s ) at %s:%d\n", \
        #lhs,                                                 \
        #rhs,                                                 \
        _lhs,                                                 \
        _rhs,                                                 \
        __FILE__,                                             \
        __LINE__);                                            \
      if (LIBD_TESTING_ABORT_ON_FAIL) {                       \
        abort();                                              \
      }                                                       \
    }                                                         \
  } while (0)

/*
 * maybe add str: NE, substring, startswith, empty, not empty
 */

/*
 * maybe add range: in range, out of range
 */

/*
 * maybe add meta: assert fail, assert success, assert failure
 * note: success/failure would be aliases.
 */

#endif  // LIBD_TESTING_H
