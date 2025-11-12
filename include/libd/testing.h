/**
 * @file libd_testing.h
 * @brief Testing for libdane.
 */

#ifndef LIBD_TESTING_H
#define LIBD_TESTING_H

#include <inttypes.h>
#include <stdarg.h>
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

#define ARR_LEN(arr) (sizeof(arr) / sizeof(arr[0]))

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

#define ASSERT_FMT_STR "Assertion failed: %%s %%s %%s ( %s vs %s ) at %%s:%%d\n"

typedef bool (*_pred_f)(
  uintptr_t lhs,
  uintptr_t rhs);

struct _predicate {
  _pred_f f;
  const char* op;
};

// clang-format off
LIBD_USED static inline bool is_eq_u(uintptr_t lhs, uintptr_t rhs) {return lhs == rhs;}
LIBD_USED static struct _predicate eq_u_pred = { .f = is_eq_u, .op = "==" };
LIBD_USED static inline bool is_ne_u(uintptr_t lhs, uintptr_t rhs) {return lhs != rhs;}
LIBD_USED static struct _predicate ne_u_pred = { .f = is_ne_u, .op = "!=" };
LIBD_USED static inline bool is_lt_u(uintptr_t lhs, uintptr_t rhs) {return lhs < rhs;}
LIBD_USED static struct _predicate lt_u_pred = { .f = is_lt_u, .op = "<" };
LIBD_USED static inline bool is_le_u(uintptr_t lhs, uintptr_t rhs) {return lhs <= rhs;}
LIBD_USED static struct _predicate le_u_pred = { .f = is_le_u, .op = "<=" };
LIBD_USED static inline bool is_gt_u(uintptr_t lhs, uintptr_t rhs) {return lhs > rhs;}
LIBD_USED static struct _predicate gt_u_pred = { .f = is_gt_u, .op = ">" };
LIBD_USED static inline bool is_ge_u(uintptr_t lhs, uintptr_t rhs) {return lhs >= rhs;}
LIBD_USED static struct _predicate ge_u_pred = { .f = is_ge_u, .op = ">=" };

LIBD_USED static inline bool is_eq_s(uintptr_t lhs, uintptr_t rhs) {return (int64_t)lhs == (int64_t)rhs;}
LIBD_USED static struct _predicate eq_s_pred = { .f = is_eq_s, .op = "==" };
LIBD_USED static inline bool is_ne_s(uintptr_t lhs, uintptr_t rhs) {return (int64_t)lhs != (int64_t)rhs;}
LIBD_USED static struct _predicate ne_s_pred = { .f = is_ne_s, .op = "!=" };
LIBD_USED static inline bool is_lt_s(uintptr_t lhs, uintptr_t rhs) {return (int64_t)lhs < (int64_t)rhs;}
LIBD_USED static struct _predicate lt_s_pred = { .f = is_lt_s, .op = "<" };
LIBD_USED static inline bool is_le_s(uintptr_t lhs, uintptr_t rhs) {return (int64_t)lhs <= (int64_t)rhs;}
LIBD_USED static struct _predicate le_s_pred = { .f = is_le_s, .op = "<=" };
LIBD_USED static inline bool is_gt_s(uintptr_t lhs, uintptr_t rhs) {return (int64_t)lhs > (int64_t)rhs;}
LIBD_USED static struct _predicate gt_s_pred = { .f = is_gt_s, .op = ">" };
LIBD_USED static inline bool is_ge_s(uintptr_t lhs, uintptr_t rhs) {return (int64_t)lhs >= (int64_t)rhs;}
LIBD_USED static struct _predicate ge_s_pred = { .f = is_ge_s, .op = ">=" };

LIBD_USED static inline bool is_eq_str(uintptr_t lhs, uintptr_t rhs) {return strcmp((char*)lhs, (char*)rhs) == 0;}
LIBD_USED static struct _predicate eq_str_pred = { .f = is_eq_str, .op = "==" };
// clang-format on

static inline void
libd_assert_op(
  struct _predicate pred,
  uintptr_t lhs,
  uintptr_t rhs,
  const char* lhs_s,
  const char* rhs_s,
  const char* repr,
  const char* file,
  int line,
  const char* opt_ctx,
  ...)
{
  char fmt[128];
  snprintf(fmt, sizeof(fmt), ASSERT_FMT_STR, repr, repr);

  if (!pred.f(lhs, rhs)) {
    fprintf(stderr, fmt, lhs_s, pred.op, rhs_s, lhs, rhs, file, line);
    if (opt_ctx != NULL) {
      va_list args;
      va_start(args, opt_ctx);
      vfprintf(stderr, opt_ctx, args);
      va_end(args);
    }
    if (LIBD_TESTING_ABORT_ON_FAIL) {
      abort();
    }
  }
}

#define ASSERT_OP(pred, lhs, rhs, repr, ...)                                   \
  libd_assert_op(                                                              \
    pred, lhs, rhs, #lhs, #rhs, repr, __FILE__, __LINE__, ##__VA_ARGS__, NULL)

#define ASSERT_TRUE(expr, ...)                               \
  ASSERT_OP(eq_u_pred, (expr), 1, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_FALSE(expr, ...)                              \
  ASSERT_OP(eq_u_pred, (expr), 0, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_ZERO(expr, ...)                               \
  ASSERT_OP(eq_u_pred, (expr), 0, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_NONZERO(expr, ...)                            \
  ASSERT_OP(ne_u_pred, (expr), 0, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_OK(expr, ...)                                 \
  ASSERT_OP(eq_u_pred, (expr), 0, "%" PRIu64, ##__VA_ARGS__)

#define ASSERT_EQ_U(lhs, rhs, ...)                          \
  ASSERT_OP(eq_u_pred, lhs, rhs, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_NE_U(lhs, rhs, ...)                          \
  ASSERT_OP(ne_u_pred, lhs, rhs, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_LT_U(lhs, rhs, ...)                          \
  ASSERT_OP(lt_u_pred, lhs, rhs, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_LE_U(lhs, rhs, ...)                          \
  ASSERT_OP(le_u_pred, lhs, rhs, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_GT_U(lhs, rhs, ...)                          \
  ASSERT_OP(gt_u_pred, lhs, rhs, "%" PRIu64, ##__VA_ARGS__)
#define ASSERT_GE_U(lhs, rhs, ...)                          \
  ASSERT_OP(ge_u_pred, lhs, rhs, "%" PRIu64, ##__VA_ARGS__)

#define ASSERT_EQ_S(lhs, rhs, ...)                          \
  ASSERT_OP(eq_s_pred, lhs, rhs, "%" PRId64, ##__VA_ARGS__)
#define ASSERT_NE_S(lhs, rhs, ...)                          \
  ASSERT_OP(ne_s_pred, lhs, rhs, "%" PRId64, ##__VA_ARGS__)
#define ASSERT_LT_S(lhs, rhs, ...)                          \
  ASSERT_OP(lt_s_pred, lhs, rhs, "%" PRId64, ##__VA_ARGS__)
#define ASSERT_LE_S(lhs, rhs, ...)                          \
  ASSERT_OP(le_s_pred, lhs, rhs, "%" PRId64, ##__VA_ARGS__)
#define ASSERT_GT_S(lhs, rhs, ...)                          \
  ASSERT_OP(gt_s_pred, lhs, rhs, "%" PRId64, ##__VA_ARGS__)
#define ASSERT_GE_S(lhs, rhs, ...)                          \
  ASSERT_OP(ge_s_pred, lhs, rhs, "%" PRId64, ##__VA_ARGS__)

#define ASSERT_EQ_PTR(lhs, rhs, ...)                  \
  ASSERT_OP(eq_u_pred, lhs, rhs, "%p", ##__VA_ARGS__)
#define ASSERT_NE_PTR(lhs, rhs, ...)                  \
  ASSERT_OP(ne_u_pred, lhs, rhs, "%p", ##__VA_ARGS__)
#define ASSERT_NULL(ptr, ...)                          \
  ASSERT_OP(eq_u_pred, ptr, NULL, "%p", ##__VA_ARGS__)
#define ASSERT_NOT_NULL(ptr, ...)                      \
  ASSERT_OP(ne_u_pred, ptr, NULL, "%p", ##__VA_ARGS__)

// #define ASSERT_EQ_MEM(lhs, rhs, len, ...)

/*
 * maybe add mem: NE, zero'd
 */

#define ASSERT_EQ_STR(lhs, rhs, ...)                                          \
  ASSERT_OP(eq_str_pred, (uintptr_t)lhs, (uintptr_t)rhs, "%s", ##__VA_ARGS__)

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
