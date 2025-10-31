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
  #define LIBD_TESTING_ABORT_ON_FAIL 0
#endif

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
    if ((memcmp((_lhs), (_rhs), (_len))) != 0) {                  \
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

#define ASSERT_EQ_STR(lhs, rhs)                               \
  do {                                                        \
    const char* _lhs = (lhs);                                 \
    const char* _rhs = (rhs);                                 \
    size_t _len      = strlen(lhs);                           \
    if ((strcmp((_lhs), (_rhs), (_len))) != 0) {              \
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

#endif  // LIBD_TESTING_H
