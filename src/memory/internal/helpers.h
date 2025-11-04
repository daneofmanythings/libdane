#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @brief Check whether or not the given offset is a power of 2.
 * @param The offset to validate.
 * @return true if power of 2, false otherwise.
 */
bool
libd_memory_is_power_of_two(size_t offset);

/**
 * @brief Validates the given alignment. Must be a non-zero power of 2 that
 * doesn't exceed the max alignment.
 * @param alignment Value to verify.
 * @return LIBD_MEM_OK on success, non-zero otherwise.
 */
enum libd_result
libd_memory_is_valid_alignment(uint8_t alignment);

/**
 * @brief Rounds the value to the nearest given alignment value.
 * @warning alignment must be non-zero.
 * @param value The value to round from.
 * @param alignment The power of 2 to round to. This parameter MUST be a power
 * of 2.
 */
uintptr_t
libd_memory_align_value(
  uintptr_t value,
  uint8_t alignment);
