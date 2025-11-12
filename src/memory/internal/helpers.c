#include "helpers.h"

#include "../../../include/libd/common.h"
#include "../../../include/libd/utils/align_compat.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Check whether or not the given offset is a power of 2.
 * @param The offset to validate.
 * @return true if power of 2, false otherwise.
 */
bool
libd_memory_is_power_of_two(size_t offset)
{
  return ((offset & (offset - 1)) == 0);
}

/**
 * @brief Validates the given alignment. Must be a non-zero power of 2 that
 * doesn't exceed the max alignment.
 * @param alignment Value to verify.
 * @return LIBD_MEM_OK on success, non-zero otherwise.
 */
bool
libd_memory_is_valid_alignment(uint8_t alignment)
{
  if (alignment == 0) {
    return false;
  }

  if (!libd_memory_is_power_of_two(alignment) || alignment > LIBD_MAX_ALIGN) {
    return false;
  }

  return true;
}

uintptr_t
libd_memory_align_value(
  uintptr_t value,
  uintptr_t alignment)
{
  return ((value + alignment - 1) & ~(uintptr_t)(alignment - 1));
}
