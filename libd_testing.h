/**
 * @file libd_testing.h
 * @brief A testing header.
 */

#ifndef LIBD_TESTING_H
#define LIBD_TESTING_H

// TODO:
#define ASSERT(expr) \
  do {               \
    if (!expr) {     \
    }                \
  } while (0)

// TODO:
#define _ASSERTING(lh, op, rh, repr) \
  do {                               \
    if (!((lh)(op)(rh))) {           \
    }                                \
  } while (0)

#define ASSERT_EQ(lh, rh, repr) \
  do {                          \
    if (!((lh) == (rh))) {      \
    }                           \
  } while (0)

#endif  // LIBD_TESTING_H
