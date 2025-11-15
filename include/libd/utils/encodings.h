/**
 * @file encodings.h
 */

#ifndef LIBD_ENCODINGS_H
#define LIBD_ENCODINGS_H

#include "../common.h"

#include <stdbool.h>
#include <string.h>

enum libd_character_encoding {
  libd_ascii,
  libd_utf8,
  libd_utf16,
  libd_utf32,
};

/**
 * @brief encoding lengths from the wiki article:
 * [https://en.wikipedia.org/wiki/UTF-8#Description]
 * @param leading_byte the first byte in utf8 encoding.
 * @return Returns the length of the character in bytes. 0 on unrecognized
 * encoding.
 */
static inline u8
libd_utf8_char_len(const u8 leading_byte)
{
  if ((leading_byte >> 7) == 0b0)
    return 1;
  if ((leading_byte >> 5) == 0b110)
    return 2;
  if ((leading_byte >> 4) == 0b1110)
    return 3;
  if ((leading_byte >> 3) == 0b11110)
    return 4;

  return 0;
}

static u8
libd_utf8_write_char(
  u8* dest,
  const u8* src)
{
  u8 len = libd_utf8_char_len(*src);
  memcpy(dest, src, len);

  return len;
}

static bool
libd_utf8_is_char_equal_to(
  const u8* subject,
  const u8* object)
{
  u8 len = libd_utf8_char_len(*subject);
  if (len == 0) {
    return false;
  }
  return memcmp(subject, object, len) == 0;
}

#endif  // LIBD_ENCODINGS_H
