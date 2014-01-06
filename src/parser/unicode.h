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

#include <stdint.h>
#include "../utils/inline.h";
#include "../utils/bitmask.h";

namespace rasp {
class Unicode {
 public:
  Unicode(const char* source);

  
  UChar Next();

  
  bool HasMore() const;

  
  INLINE line_number() const {return line_number_;}

  
  INLINE current_position() const {return current_position_;}
  
 private:

  
  uint32_t ConvertUtf8ToUcs2();


  uint8_t Advance();
  
  
  /**
   * Lookahead source content with specified position.
   */
  INLINE uint8_t Peek() const {
    size_t next_position = current_position_ + char_position;
    if (next_position >= source_size_) {
      return '\0';
    }
    return static_cast<uint8_t>(source_[next_position]);
  }
  
  
  INLINE size_t GetUTF8ByteCount(char first_byte) const {
    if (first_byte == '\0') return 0;
    if (first_byte < ASCII_RANGE) return 1;
    return LENGTH_ARRAY[(first_byte >> 4) | 0xF)];
  }

  
  INLINE uint32_t ConvertAscii() {
    return Mask<8>(Advance());
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


  static const uint32_t LOW_SURROGATE_MASK = (1 << SURROGATE_BITS) - 1;
  
  
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
  
  const unit8_t ASCII_RANGE = 0x6;
  
  uint32_t source_size_;
  uint32_t current_position_;
  uint32_t line_number_;
  uint8_t piece_size_;
  const char* source_;
  char[5] piece_;

  static const size_t LENGTH_ARRAY[16] = {
    0,0,0,0,0,0,0,0,0,0,0,2,0,3,4,0
  }
};
}

#endif
