#ifndef LIBD_ALIGN_COMPAT_H
#define LIBD_ALIGN_COMPAT_H

#include <stddef.h>

/*
 * Cross-version, cross-compiler alignment compatibility layer.
 * Provides:
 *   - LIBD_ALIGNOF(type): yields the alignment requirement of 'type'
 *   - LIBD_MAX_ALIGN_T: the most strictly aligned scalar type
 */

#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
/* --- C11 and newer --- */
#include <stdalign.h>
#define LIBD_ALIGNOF _Alignof
#define LIBD_MAX_ALIGN_T max_align_t

#else
/* --- Pre-C11 (e.g., C99) --- */
#if defined(__GNUC__) || defined(__clang__)
#define LIBD_ALIGNOF __alignof__
#elif defined(_MSC_VER)
#define LIBD_ALIGNOF __alignof
#else
#error "No supported alignof equivalent for this compiler"
#endif

/* Define a conservative fallback for max alignment.
   This covers most platforms and is safe for heap allocators. */
typedef union {
  long long ll;
  long double ld;
  void* p;
} _libd_max_align_t;

#define LIBD_MAX_ALIGN_T _libd_max_align_t
#endif

#define LIBD_MAX_ALIGN (LIBD_ALIGNOF(LIBD_MAX_ALIGN_T))

#endif /* LIBD_ALIGN_COMPAT_H */
