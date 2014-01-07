/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2013 Taketoshi Aono(brn)
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef PARSER_UNICODE_H_
#define PARSER_UNICODE_H_

#include <array>
#include <stdint.h>
#include "../utils/inline.h"
#include "../utils/bitmask.h"
#include "./uchar.h"

namespace rasp {
class Unicode {
 public:
  Unicode(const char* source);

  Unicode(const Unicode& un) = delete;

  Unicode(Unicode&& un) = delete;

  ~Unicode() = default;
  
  UChar Next();

  
  INLINE bool HasMore() const {return Peek() != '\0';}

  
  INLINE uint32_t line_number() const {return line_number_;}

  
  INLINE uint32_t current_position() const {return current_position_;}
  
 private:

  
  uint32_t ConvertUtf8ToUcs2(size_t byte_count);


  uint8_t Advance();
  
  
  /**
   * Lookahead source content with specified position.
   */
  INLINE uint8_t Peek(int char_position = 1) const {
    size_t next_position = current_position_ + char_position;
    if (next_position >= source_size_) {
      return '\0';
    }
    return static_cast<uint8_t>(source_[next_position]);
  }

  
  template<std::size_t N, typename CharT>
  INLINE CharT Mask(CharT ch) {
    return Bitmask<N, uint32_t>::lower & ch;
  }
  
  
  INLINE size_t GetUTF8ByteCount(uint8_t first_byte) const {
    static const std::array<uint8_t, UINT8_MAX + 1> kLengthMap = { {
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // 00000000 -> 00011111
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // 00100000 -> 00111111
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // 01000000 -> 01011111
        1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,  // 01100000 -> 01111111  ASCII range end
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 10000000 -> 10011111  invalid
        0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,  // 10100000 -> 10111111  invalid
        2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,  // 11000000 -> 11011111  2 bytes range
        3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,                                  // 11100000 -> 11101111  3 bytes range
        4,4,4,4,4,4,4,4,                                                  // 11110000 -> 11110111  4 bytes range
        0,0,0,0,0,0,0,0                                                   // 11111000 -> 11111111  invalid
      } };
    return kLengthMap[first_byte];
  }

  
  INLINE uint32_t ConvertAscii() {
    return Mask<8>(Append(Advance()));
  }

  
  INLINE bool IsTrail(char c) {
    return (c & 0xC0) == 0x80;
  }

  
  uint32_t Convert2Byte();

  
  uint32_t Convert3Byte();

  
  uint32_t Convert4Byte();

  
  static const uint32_t SURROGATE_BITS = 10;

  
  static const uint32_t HIGH_SURROGATE_MIN = 0xD800;

  
  static const uint32_t HIGH_SURROGATE_MAX = 0xDBFF;

  
  static const uint32_t HIGH_SURROGATE_OFFSET = HIGH_SURROGATE_MIN - (0x10000 >> 10);

  
  static const uint32_t LOW_SURROGATE_MIN = 0xDC00;


  static const uint32_t LOW_SURROGATE_MAX = 0xDFFF;


  static const uint32_t LOW_SURROGATE_MASK = (1 << SURROGATE_BITS) - 1;


  static const uint32_t SURROGATE_MAX = 0xFFFF;
  
  
  INLINE uint16_t ToHighSurrogate(uint32_t uc) {
    return static_cast<uint16_t>((uc >> SURROGATE_BITS) + HIGH_SURROGATE_OFFSET);
  }

  
  INLINE uint16_t ToLowSurrogate(uint32_t uc) {
    return static_cast<uint16_t>((uc & LOW_SURROGATE_MASK) + LOW_SURROGATE_MIN);
  }


  INLINE char Append(char c) {
    piece_[piece_size_] = c;
    piece_size_++;
    return c;
  }
  
  const uint8_t ASCII_RANGE = 0x7F;
  
  uint32_t source_size_;
  uint32_t current_position_;
  uint32_t line_number_;
  uint8_t piece_size_;
  const char* source_;
  char piece_[5];

  static const size_t LENGTH_ARRAY[16];
};
}

#endif
