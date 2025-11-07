/**
 * @file libd_testing.h
 * @brief Testing for libdane.
 */

#ifndef LIBD_TESTING_H
#define LIBD_TESTING_H

#include <inttypes.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef _LIBD_TYPES
  #include <stdint.h>
#endif

#ifndef LIBD_TESTING_ABORT_ON_FAIL
  #define LIBD_TESTING_ABORT_ON_FAIL 1
#endif

#ifndef LIBD_USED
  #if defined(__GNUC__) || defined(__clang__)
    #define LIBD_USED __attribute__((used))
  #elif defined(_MSC_VER)
    /* MSVC doesn't have a direct equivalent; force reference retention. */
    #define LIBD_USED __pragma(warning(suppress : 4505))
  #else
    #define LIBD_USED /* no-op on unknown compilers */
  #endif
#endif

enum libd_test_code {
  libd_test_ok,
  libd_test_fail,
  libd_test_skipped,
  //
  libd_test_result_count,
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
  LIBD_USED                                            \
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

#define _ASSERT_COUNT(                                                 \
  _1, _2, _3, _4, _5, _6, _7, _8, _9, _A, _B, _C, _D, _E, _F, _N, ...) \
  _N
#define _ASSERT_ARGN(...)                                                    \
  _ASSERT_COUNT(__VA_ARGS__, F, E, D, C, B, A, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0)
#define _ASSERT_CAT0(x, y) x##y
#define _ASSERT_CAT1(x, y) _ASSERT_CAT0(x, y)
#define _ASSERT_DISPATCH(f, ...)                          \
  _ASSERT_CAT1(f, _ASSERT_ARGN(__VA_ARGS__))(__VA_ARGS__)

#define ASSERT_OP_IMPL(lhs, op, rhs, type, repr, has_fmt, ...)          \
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
      if (has_fmt) {                                                    \
        fprintf(stderr, __VA_ARGS__);                                   \
      }                                                                 \
      if (LIBD_TESTING_ABORT_ON_FAIL) {                                 \
        abort();                                                        \
      }                                                                 \
    }                                                                   \
  } while (0)

#define ASSERT_OP_5(lhs, op, rhs, type, repr) \
  ASSERT_OP_IMPL(lhs, op, rhs, type, repr, 0)

#define ASSERT_OP_6(lhs, op, rhs, type, repr, fmt) \
  ASSERT_OP_IMPL(lhs, op, rhs, type, repr, 1, fmt)

#define ASSERT_OP_7(lhs, op, rhs, type, repr, fmt, a1) \
  ASSERT_OP_IMPL(lhs, op, rhs, type, repr, 1, fmt, a1)

#define ASSERT_OP_8(lhs, op, rhs, type, repr, fmt, a1, a2) \
  ASSERT_OP_IMPL(lhs, op, rhs, type, repr, 1, fmt, a1, a2)

#define ASSERT_OP_9(lhs, op, rhs, type, repr, fmt, a1, a2, a3) \
  ASSERT_OP_IMPL(lhs, op, rhs, type, repr, 1, fmt, a1, a2, a3)

#define ASSERT_OP_A(lhs, op, rhs, type, repr, fmt, a1, a2, a3, a4) \
  ASSERT_OP_IMPL(lhs, op, rhs, type, repr, 1, fmt, a1, a2, a3, a4)

#define ASSERT_OP_B(lhs, op, rhs, type, repr, fmt, a1, a2, a3, a4, a5) \
  ASSERT_OP_IMPL(lhs, op, rhs, type, repr, 1, fmt, a1, a2, a3, a4, a5)

#define ASSERT_OP_C(lhs, op, rhs, type, repr, fmt, a1, a2, a3, a4, a5, a6) \
  ASSERT_OP_IMPL(lhs, op, rhs, type, repr, 1, fmt, a1, a2, a3, a4, a5, a6)

#define ASSERT_OP_D(lhs, op, rhs, type, repr, fmt, a1, a2, a3, a4, a5, a6, a7) \
  ASSERT_OP_IMPL(lhs, op, rhs, type, repr, 1, fmt, a1, a2, a3, a4, a5, a6, a7)

#define ASSERT_OP_E(                                                  \
  lhs, op, rhs, type, repr, fmt, a1, a2, a3, a4, a5, a6, a7, a8)      \
  ASSERT_OP_IMPL(                                                     \
    lhs, op, rhs, type, repr, 1, fmt, a1, a2, a3, a4, a5, a6, a7, a8)

#define ASSERT_OP_F(                                                      \
  lhs, op, rhs, type, repr, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9)      \
  ASSERT_OP_IMPL(                                                         \
    lhs, op, rhs, type, repr, 1, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9)

#define ASSERT_OP(...) _ASSERT_DISPATCH(ASSERT_OP_, __VA_ARGS__)

#define ASSERT_TRUE(expr, ...)  ASSERT_OP(expr, ==, 1, int, "%d", ##__VA_ARGS__)
#define ASSERT_FALSE(expr, ...) ASSERT_OP(expr, ==, 0, int, "%d", ##__VA_ARGS__)
#define ASSERT_ZERO(expr, ...)  ASSERT_OP(expr, ==, 0, int, "%d", ##__VA_ARGS__)
#define ASSERT_NONZERO(expr, ...)                  \
  ASSERT_OP(expr, !=, 0, int, "%d", ##__VA_ARGS__)
#define ASSERT_OK(expr, ...) ASSERT_OP(expr, ==, 0, int, "%d", ##__VA_ARGS__)

#define ASSERT_EQ_U(lhs, rhs, ...)                             \
  ASSERT_OP(lhs, ==, rhs, uint64_t, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_NE_U(lhs, rhs, ...)                             \
  ASSERT_OP(lhs, !=, rhs, uint64_t, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_LT_U(lhs, rhs, ...)                            \
  ASSERT_OP(lhs, <, rhs, uint64_t, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_LE_U(lhs, rhs, ...)                             \
  ASSERT_OP(lhs, <=, rhs, uint64_t, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_GT_U(lhs, rhs, ...)                            \
  ASSERT_OP(lhs, >, rhs, uint64_t, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_GE_U(lhs, rhs, ...)                             \
  ASSERT_OP(lhs, >=, rhs, uint64_t, "%" PRIu64, ##__VA_ARGS__)

#define ASSERT_EQ_S(lhs, rhs, ...)                            \
  ASSERT_OP(lhs, ==, rhs, int64_t, "%" PRId64, ##__VA_ARGS__)
#define ASSERT_NE_S(lhs, rhs, ...)                            \
  ASSERT_OP(lhs, !=, rhs, int64_t, "%" PRId64, ##__VA_ARGS__)
#define ASSERT_LT_S(lhs, rhs, ...)                           \
  ASSERT_OP(lhs, <, rhs, int64_t, "%" PRId64, ##__VA_ARGS__)
#define ASSERT_LE_S(lhs, rhs, ...)                             \
  ASSERT_OP(lhs, <=, rhs, int64_t, "%", PRId64, ##__VA_ARGS__)
#define ASSERT_GT_S(lhs, rhs, ...)                           \
  ASSERT_OP(lhs, >, rhs, int64_t, "%" PRId64, ##__VA_ARGS__)
#define ASSERT_GE_S(lhs, rhs, ...)                            \
  ASSERT_OP(lhs, >=, rhs, int64_t, "%" PRId64, ##__VA_ARGS__)

#define ASSERT_EQ_PTR(lhs, rhs, ...)                  \
  ASSERT_OP(lhs, ==, rhs, void*, "%p", ##__VA_ARGS__)
#define ASSERT_NE_PTR(lhs, rhs, ...)                  \
  ASSERT_OP(lhs, !=, rhs, void*, "%p", ##__VA_ARGS__)
#define ASSERT_NULL(ptr, ...)     ASSERT_EQ_PTR(ptr, NULL, ##__VA_ARGS__)
#define ASSERT_NOT_NULL(ptr, ...) ASSERT_NE_PTR(ptr, NULL, ##__VA_ARGS__)

#define ASSERT_MEM_IMPL(pred, lhs, rhs, len, has_fmt, ...)        \
  do {                                                            \
    const void* _lhs = (void*)(lhs);                              \
    const void* _rhs = (void*)(rhs);                              \
    size_t _len      = (len);                                     \
    if (!(pred(_lhs, _rhs, _len))) {                              \
      fprintf(                                                    \
        stderr,                                                   \
        "Assertion failed: (%s) != (%s) ( %p vs %p ) at %s:%d\n", \
        #lhs,                                                     \
        #rhs,                                                     \
        _lhs,                                                     \
        _rhs,                                                     \
        __FILE__,                                                 \
        __LINE__);                                                \
      if (has_fmt) {                                              \
        fprintf(stderr, __VA_ARGS__);                             \
      }                                                           \
      if (LIBD_TESTING_ABORT_ON_FAIL) {                           \
        abort();                                                  \
      }                                                           \
    }                                                             \
  } while (0)

#define ASSERT_MEM_4(pred, lhs, rhs, len) \
  ASSERT_MEM_IMPL(pred, lhs, rhs, len, 0)

#define ASSERT_MEM_5(pred, lhs, rhs, len, fmt) \
  ASSERT_MEM_IMPL(pred, lhs, rhs, len, 1, fmt)

#define ASSERT_MEM_6(pred, lhs, rhs, len, fmt, a1) \
  ASSERT_MEM_IMPL(pred, lhs, rhs, len, 1, fmt, a1)

#define ASSERT_MEM_7(pred, lhs, rhs, len, fmt, a1, a2) \
  ASSERT_MEM_IMPL(pred, lhs, rhs, len, 1, fmt, a1, a2)

#define ASSERT_MEM_8(pred, lhs, rhs, len, fmt, a1, a2, a3) \
  ASSERT_MEM_IMPL(pred, lhs, rhs, len, 1, fmt, a1, a2, a3)

#define ASSERT_MEM_9(pred, lhs, rhs, len, fmt, a1, a2, a3, a4) \
  ASSERT_MEM_IMPL(pred, lhs, rhs, len, 1, fmt, a1, a2, a3, a4)

#define ASSERT_MEM_A(pred, lhs, rhs, len, fmt, a1, a2, a3, a4, a5) \
  ASSERT_MEM_IMPL(pred, lhs, rhs, len, 1, fmt, a1, a2, a3, a4, a5)

#define ASSERT_MEM_B(pred, lhs, rhs, len, fmt, a1, a2, a3, a4, a5, a6) \
  ASSERT_MEM_IMPL(pred, lhs, rhs, len, 1, fmt, a1, a2, a3, a4, a5, a6)

#define ASSERT_MEM_C(pred, lhs, rhs, len, fmt, a1, a2, a3, a4, a5, a6, a7) \
  ASSERT_MEM_IMPL(pred, lhs, rhs, len, 1, fmt, a1, a2, a3, a4, a5, a6, a7)

#define ASSERT_MEM_D(pred, lhs, rhs, len, fmt, a1, a2, a3, a4, a5, a6, a7, a8) \
  ASSERT_MEM_IMPL(pred, lhs, rhs, len, 1, fmt, a1, a2, a3, a4, a5, a6, a7, a8)

#define ASSERT_MEM_E(                                                \
  pred, lhs, rhs, len, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9)      \
  ASSERT_MEM_IMPL(                                                   \
    pred, lhs, rhs, len, 1, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9)

#define ASSERT_MEM(...) _ASSERT_DISPATCH(ASSERT_MEM_, __VA_ARGS__)

static inline bool
_is_mem_equal(
  const void* mem1,
  const void* mem2,
  size_t len)
{
  return memcmp(mem1, mem2, len) == 0;
}

#define ASSERT_EQ_MEM(lhs, rhs, len, ...)                 \
  ASSERT_MEM(_is_mem_equal, lhs, rhs, len, ##__VA_ARGS__)

/*
 * maybe add mem: NE, zero'd
 */

#define ASSERT_STR_IMPL(pred, lhs, rhs, has_fmt, ...)         \
  do {                                                        \
    const char* _lhs = (lhs);                                 \
    const char* _rhs = (rhs);                                 \
    if (!pred(_lhs, _rhs)) {                                  \
      fprintf(                                                \
        stderr,                                               \
        "Assertion failed: %s != %s ( %s vs %s ) at %s:%d\n", \
        #lhs,                                                 \
        #rhs,                                                 \
        _lhs,                                                 \
        _rhs,                                                 \
        __FILE__,                                             \
        __LINE__);                                            \
      if (has_fmt) {                                          \
        fprintf(stderr, __VA_ARGS__);                         \
      }                                                       \
      if (LIBD_TESTING_ABORT_ON_FAIL) {                       \
        abort();                                              \
      }                                                       \
    }                                                         \
  } while (0)

#define ASSERT_STR_3(pred, lhs, rhs) ASSERT_STR_IMPL(pred, lhs, rhs, 0)

#define ASSERT_STR_4(pred, lhs, rhs, fmt) \
  ASSERT_STR_IMPL(pred, lhs, rhs, 1, fmt)

#define ASSERT_STR_5(pred, lhs, rhs, fmt, a1) \
  ASSERT_STR_IMPL(pred, lhs, rhs, 1, fmt, a1)

#define ASSERT_STR_6(pred, lhs, rhs, fmt, a1, a2) \
  ASSERT_STR_IMPL(pred, lhs, rhs, 1, fmt, a1, a2)

#define ASSERT_STR_7(pred, lhs, rhs, fmt, a1, a2, a3) \
  ASSERT_STR_IMPL(pred, lhs, rhs, 1, fmt, a1, a2, a3)

#define ASSERT_STR_8(pred, lhs, rhs, fmt, a1, a2, a3, a4) \
  ASSERT_STR_IMPL(pred, lhs, rhs, 1, fmt, a1, a2, a3, a4)

#define ASSERT_STR_9(pred, lhs, rhs, fmt, a1, a2, a3, a4, a5) \
  ASSERT_STR_IMPL(pred, lhs, rhs, 1, fmt, a1, a2, a3, a4, a5)

#define ASSERT_STR_A(pred, lhs, rhs, fmt, a1, a2, a3, a4, a5, a6) \
  ASSERT_STR_IMPL(pred, lhs, rhs, 1, fmt, a1, a2, a3, a4, a5, a6)

#define ASSERT_STR_B(pred, lhs, rhs, fmt, a1, a2, a3, a4, a5, a6, a7) \
  ASSERT_STR_IMPL(pred, lhs, rhs, 1, fmt, a1, a2, a3, a4, a5, a6, a7)

#define ASSERT_STR_C(pred, lhs, rhs, fmt, a1, a2, a3, a4, a5, a6, a7, a8) \
  ASSERT_STR_IMPL(pred, lhs, rhs, 1, fmt, a1, a2, a3, a4, a5, a6, a7, a8)

#define ASSERT_STR_D(pred, lhs, rhs, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9) \
  ASSERT_STR_IMPL(pred, lhs, rhs, 1, fmt, a1, a2, a3, a4, a5, a6, a7, a8, a9)

#define ASSERT_STR(...) _ASSERT_DISPATCH(ASSERT_STR_, __VA_ARGS__)

static inline bool
_is_str_eq(
  const char* s1,
  const char* s2)
{
  return strcmp(s1, s2) == 0;
}

#define ASSERT_STR_EQ(lhs, rhs, ...)              \
  ASSERT_STR(_is_str_eq, lhs, rhs, ##__VA_ARGS__)

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
