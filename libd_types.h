/**
 * @file libd_types.h
 * @brief Reusable type alias and result codes.
 * @warning This header assumes two's compliment for integet representation.
 */

#ifndef LIBD_TYPES_H
#define LIBD_TYPES_H

#include <stdint.h>

//==============================================================================
//  Integer type definitions and min/max defines.
//==============================================================================

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

#define _UNSIGNED_MAX(n) ((u##n)0 - 1)
#define _SIGNED_MIN(n)   ((i##n)1 << (n - 1))
#define _SIGNED_MAX(n)   (~_SIGNED_MIN(n))

#define U8_MAX  _UNSIGNED_MAX(8)
#define U16_MAX _UNSIGNED_MAX(16)
#define U32_MAX _UNSIGNED_MAX(32)
#define U64_MAX _UNSIGNED_MAX(64)

#define I8_MIN  _SIGNED_MIN(8)
#define I16_MIN _SIGNED_MIN(16)
#define I32_MIN _SIGNED_MIN(32)
#define I64_MIN _SIGNED_MIN(64)

#define I8_MAX  _SIGNED_MAX(8)
#define I16_MAX _SIGNED_MAX(16)
#define I32_MAX _SIGNED_MAX(32)
#define I64_MAX _SIGNED_MAX(64)

//=============================================================================
//  Libdane result codes
//=============================================================================

enum libd_result {
  libd_ok,
  libd_err,
  libd_no_memory,
  libd_buffer_overflow,
  libd_invalid_parameter,
  libd_init_failed,
  libd_not_initialized,

  libd_invalid_path, /**< The path provided is invalid */
  libd_invalid_path_type,
  libd_env_var_not_found,
  libd_too_many_env_expansions,

  libd_invalid_alignment, /**< An invalid alignment value was given */
  libd_invalid_pointer,   /**< An invalid pointer parameter was detected */
  libd_index_must_be_unsigned,
  libd_invalid_free, /**< Tried to free memory from an arena that has no
                       allocations */

  //
  libd_mem_not_implemented, /**< Functionality not yet implemented */
  libd_result_count,
};

#endif  // LIBD_TYPES_H
