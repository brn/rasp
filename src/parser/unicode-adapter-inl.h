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

#ifndef PARSER_UNICODE_INL_H_
#define PARSER_UNICODE_INL_H_

namespace rasp {
template <typename InputIterator>
UnicodeAdapter<InputIterator>::UnicodeAdapter(InputIterator begin)
    : current_position_(0),
      line_number_(1),
      begin_(begin),
      surrogate_(0){}


template <typename InputIterator>
template <typename T>
UnicodeAdapter<InputIterator>::UnicodeAdapter(const UnicodeAdapter<T>& it)
    : current_position_(it.current_position_),
      line_number_(it.line_number_),
      begin_(it.begin_),
      surrogate_(it.surrogate_){}


template <typename InputIterator>
template <typename T>
UnicodeAdapter<InputIterator>::UnicodeAdapter(UnicodeAdapter<T>&& it)
    : current_position_(it.current_position_),
      line_number_(it.line_number_),
      begin_(std::move(it.begin_)),
      surrogate_(0){}


template <typename InputIterator>
UChar UnicodeAdapter<InputIterator>::Next () {
  if (surrogate_) {
    uint32_t surrogate = surrogate_;
    surrogate_ = 0;
    return UChar(surrogate, false);
  }
  size_t byte_count = GetUTF8ByteCount(*begin_);
  uint32_t next = ConvertUtf8ToUcs2(byte_count);
  if (next > SURROGATE_MAX) {
    // surrogate pair only ch > 0xFFFF
    // because NextUCS4FromUTF8 checks code point is valid
    surrogate_ = ToLowSurrogate(next);
    return UChar(ToHighSurrogate(next), false);
  } else if (next != 0) {
    return UChar(static_cast<uint16_t>(next), byte_count == 1);
  }
  return UChar();
}


template <typename InputIterator>
uint32_t UnicodeAdapter<InputIterator>::ConvertUtf8ToUcs2(size_t byte_count) {
  switch (byte_count) {
    case 1:
      return ConvertAscii();
    case 2:
      return Convert2Byte();
    case 3:
      return Convert3Byte();
    case 4:
      return Convert4Byte();
    default:
      return 0;
  }
}


template <typename InputIterator>
uint32_t UnicodeAdapter<InputIterator>::Convert2Byte() {
  const uint8_t MINIMUN_RANGE = 0x00000080;
  char c = *begin_;
  if (c != '\0') {
    uint32_t next = Mask<5>(c) << 6;
    c = *(begin_ + 1);
    if (c != '\0') {
      if (IsTrail(c)) {
        next = next | Mask<6>(c);
        if (next > MINIMUN_RANGE) {
          return next;
        }
      }
    }
  }
  return 0;
}


template <typename InputIterator>
uint32_t UnicodeAdapter<InputIterator>::Convert3Byte() {
  const int MINIMUN_RANGE = 0x00000800;
  char c = *begin_;
  if (c != '\0') {
    uint32_t next = Mask<4>(c) << 12;
    c = *(begin_ + 1);
    if (c != '\0') {
      if (IsTrail(c)) {
        next = next | Mask<6>(c) << 6;
        c = *(begin_ + 2);
        if (c != '\0') {
          if (IsTrail(c)) {
            next = next | Mask<6>(c);
            if (next > MINIMUN_RANGE) {
              if (next < HIGH_SURROGATE_MIN || LOW_SURROGATE_MAX < next) {
                return next;
              }
            }
          }
        }
      }
    }
  }
  return 0;
}


template <typename InputIterator>
uint32_t UnicodeAdapter<InputIterator>::Convert4Byte() {
  const int MINIMUN_RANGE = 0x000010000;
  char c = *begin_;
  if (c != '\0') {
    uint32_t next = Mask<3>(c) << 18;
    c = *(begin_ + 1);
    if (c != '\0') {
      if (IsTrail(c)) {
        next = next | Mask<6>(c) << 12;
        c = *(begin_ + 2);
        if (c != '\0') {
          if (IsTrail(c)) {
            next = next | Mask<6>(c) << 6;
            c = *(begin_ + 3);
            if (c != '\0') {
              if (IsTrail(c)) {
                next = next | Mask<6>(c);
                if (next >= MINIMUN_RANGE) {
                  if (next <= 0x10FFFF) {
                    return next;
                  }
                }
              }
            }
          }
        }
      }
    }
  }
  return 0;
}


template <typename InputIterator>
const size_t UnicodeAdapter<InputIterator>::LENGTH_ARRAY[16] = {
  0,0,0,0,0,0,0,0,0,0,0,0,2,0,3,4
};
}

#endif
