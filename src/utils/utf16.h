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

#ifndef UTILS_UNICODE_H_
#define UTILS_UNICODE_H_

#include "./inline.h"
#include "./class-trait.h"

namespace rasp {
typedef uint32_t U32;
typedef uint8_t U8;

class Unicode : private Static {
 public:
  static const UC32 kSurrogateMax = 0xFFFF;  
  static const UC32 kSurrogateBits = 10;  
  static const UC32 kHightSurrogateMin = 0xD800;  
  static const UC32 kHightSurrogateMax = 0xDBFF;  
  static const UC32 kHightSurrogateOffset = kHightSurrogateMin - (0x10000 >> 10);  
  static const UC32 kLowSurrogateMin = 0xDC00;
  static const UC32 kLowSurrogateMax = 0xDFFF;
  static const UC32 kLowSurrogateMask = (1 << SURROGATE_BITS) - 1;  
}

class Utf16 : private Static {
 public:
  INLINE static U32 IsSurrogatePair(U32 uc) {
    return uc > Unicode::kSurrogateMax;
  }
  
  INLINE static U32 ToHighSurrogate(U32 uc) {
    return (uc >> Unicode::kSurrogateBits) + Unicode::kHightSurrogateOffset;
  }

  
  INLINE static U32 ToLowSurrogate(U32 uc) {
    return (uc & Unicode::kLowSurrogateMask) + Unicode::kLowSurrogateMin;
  }

  
  INLINE static bool IsOutOfSurrogateRange(UC32 uc) {
    return uc < Unicode::kHightSurrogateMin || Unicode::kLowSurrogateMax < uc;
  }
};


class Utf8 : private Static {
 public :    
  INLINE static size_t GetByteCount(UC8 uc) {
    static const std::array<UC8, UINT8_MAX + 1> kLengthMap = { {
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


  INLINE static bool IsValidSequence(UC8 uc) {
    return IsNotNull(uc) && (c & 0xC0) == 0x80;
  }
    

  INLINE static bool IsNotNull(UC8 uc) {
    return uc != '\0';
  }


  INLINE bool IsAscii(UC8 uc) {return uc < kAsciiMax;}
  
 private:
  static const UC8 kAsciiMax = 0x7F;
}
}

#endif
