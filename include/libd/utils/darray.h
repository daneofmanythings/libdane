/**
 * @file utils/darray.h
 */

#ifndef LIBD_UTILS_DARRAY_H
#define LIBD_UTILS_DARRAY_H

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define DEFINE_LIBD_DARRAY_HEADER(type)                             \
  typedef struct libd_##type_darray libd_##type_darray_t;           \
                                                                    \
  libd_##type_darray_t* libd_##type_darray_create(size_t capacity); \
  void libd_##type_darray_destroy(libd_##type_darray_t* arr);       \
  const type* libd_##type_darray_at(                                \
    const libd_##type_darray_t* arr, size_t i);                     \
  int libd_##type_darray_append_n(                                  \
    libd_##type_darray_t* restrict arr, const type* dat, size_t n); \
  int libd_##type_darray_append(                                    \
    libd_##type_darray_t* restrict arr, const type* dat);           \
  int libd_##type_darray_insert_at(                                 \
    libd_##type_darray_t* restrict arr, size_t i, const type* dat);

#define DEFINE_LIBD_DARRAY_IMPL(type)                                   \
  struct libd_##type_darray {                                           \
    size_t capacity;                                                    \
    size_t size;                                                        \
    type* dat;                                                          \
  };                                                                    \
                                                                        \
  struct libd_##type_darray* libd_##type_darray_create(size_t capacity) \
  {                                                                     \
    if (capacity == 0) {                                                \
      return NULL;                                                      \
    }                                                                   \
    struct libd_##type_darray* arr = malloc(sizeof(*arr));              \
    if (arr == NULL) {                                                  \
      return NULL;                                                      \
    }                                                                   \
    arr->dat = malloc(sizeof(type) * capacity);                         \
    if (arr->dat == NULL) {                                             \
      free(arr);                                                        \
      return NULL;                                                      \
    }                                                                   \
    arr->capacity = capacity;                                           \
    arr->size     = 0;                                                  \
                                                                        \
    return arr;                                                         \
  }                                                                     \
  const type* libd_##type_darray_at(                                    \
    const struct libd_##type_darray* arr, size_t i)                     \
  {                                                                     \
    if (arr == NULL || arr->dat == NULL) {                              \
      return NULL;                                                      \
    }                                                                   \
    if (i >= arr->size) {                                               \
      return NULL;                                                      \
    }                                                                   \
    return &arr->dat[i];                                                \
  }                                                                     \
  int libd_##type_darray_memcpy_at(                                     \
    struct libd_##type_darray* restrict arr,                            \
    size_t i,                                                           \
    const type* dat,                                                    \
    size_t n)                                                           \
  {                                                                     \
    if (arr == NULL || arr->dat == NULL || dat == NULL) {               \
      return -3;                                                        \
    }                                                                   \
    if (i > arr->size) {                                                \
      return -2;                                                        \
    }                                                                   \
    size_t new_cap = arr->capacity;                                     \
    while (i + n > new_cap) {                                           \
      new_cap *= 2;                                                     \
    }                                                                   \
    if (new_cap != arr->capacity) {                                     \
      type* r = realloc(arr->dat, new_cap * sizeof(type));              \
      if (r == NULL) {                                                  \
        return -1;                                                      \
      }                                                                 \
      arr->dat      = r;                                                \
      arr->capacity = new_cap;                                          \
    }                                                                   \
    memcpy(&arr->dat[i], dat, n * sizeof(type));                        \
    arr->size = MAX(i + n, arr->size);                                  \
    return 0;                                                           \
  }                                                                     \
  int libd_##type_darray_append_n(                                      \
    struct libd_##type_darray* restrict arr, const type* dat, size_t n) \
  {                                                                     \
    if (arr == NULL) {                                                  \
      return -3;                                                        \
    }                                                                   \
    return libd_##type_darray_memcpy_at(arr, arr->size, dat, n);        \
  }                                                                     \
  int libd_##type_darray_append(                                        \
    struct libd_##type_darray* restrict arr, const type* dat)           \
  {                                                                     \
    return libd_##type_darray_append_n(arr, dat, 1);                    \
  }                                                                     \
  int libd_##type_darray_insert_at(                                     \
    struct libd_##type_darray* restrict arr, size_t i, const type* dat) \
  {                                                                     \
    return libd_##type_darray_memcpy_at(arr, i, dat, 1);                \
  }                                                                     \
  void libd_##type_darray_destroy(struct libd_##type_darray* arr)       \
  {                                                                     \
    if (arr != NULL) {                                                  \
      if (arr->dat != NULL) {                                           \
        free(arr->dat);                                                 \
      }                                                                 \
      free(arr);                                                        \
    }                                                                   \
  }

#endif  // LIBD_UTILS_DARRAY_H
