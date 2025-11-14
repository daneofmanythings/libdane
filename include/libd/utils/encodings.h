/**
 * @file encodings.h
 */

#ifndef LIBD_ENCODINGS_H
#define LIBD_ENCODINGS_H

#include "../common.h"

enum libd_string_encoding {
  libd_ascii,
  libd_utf8,
  libd_utf16,
};

/**
 * @brief encoding lengths from the wiki article:
 * [https://en.wikipedia.org/wiki/UTF-8#Description]
 * @param leading_byte the first byte in utf8 encoding.
 * @return Returns the length of the character in bytes. 0 on unrecognized
 * encoding.
 */
static inline u8
libd_utf8_char_len(u8 leading_byte)
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

#endif  // LIBD_ENCODINGS_H
