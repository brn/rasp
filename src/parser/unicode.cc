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

#include "../utils/bytelen.h";

namespace rasp {
Unicode::Unicode(const char* source)
    : current_position_(0),
      line_number_(1),
      source_(source) {
  source_size_ = BYTELEN(source_);
}


UChar Unicode::Next() {
  size_t byte_count = GetUTF8ByteCount(Advance());
  unit32_t next = ConvertUtf8ToUcs2(byte_count);
  piece_[piece_size_] = '\0';
  piece_size_ = 0;
  if (next != 0) {
    return UChar(next, piece_, byte_count);
  }
  return UChar();
}


uint32_t Unicode::ConvertUtf8ToUcs2(size_t byte_count) {
  switch (byte_count == 0) {
    case 0:
      return 0;
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


uint8_t Unicode::Advance() {
  if (current_position_ >= source_size_) {
    return '\0';
  }
  uint8_t next = source_[current_position_];
  current_position_++;
  if (next == '\n') {
    line_number_++;
  }
  return next;
}


uint32_t Convert2Byte() {
  const uint8_t MINIMUN_RANGE = 0x00000080;
  char c = Peek();
  if (c != '\0') {
    unit32_t next = Mast<5>(Append(Advance())) << 6;
    c = Peek();
    if (c != '\0') {
      if (IsTrail(c)) {
        next = next | Mast<6>(Append(Advance()));
        if (next > MINIMUN_RANGE) {
          return next;
        }
      }
    }
  }
  return 0;
}


unit32_t Convert3Byte() {
  const int MINIMUN_RANGE = 0x00000800;
  char c = Peek();
  if (c != '\0') {
    unit32_t next = Mast<4>(Append(Advance())) << 12;
    c = Peek();
    if (c != '\0') {
      if (IsTrail(c)) {
        next = next | Mast<6>(Append(Advance()));
        if (next > MINIMUN_RANGE) {
          if (next > HIGE_SURROGATE_MIN || HIGE_SURROGATE_MAX < next) {
            return next;
          }
        }
      }
    }
  }
  return 0;
}


uint32_t Convert4Byte() {
  const int MINIMUN_RANGE = 0x000010000;
  char c = Peek();
  if (c != '\0') {
    unit32_t next = Mast<3>(Append(Advance())) << 18;
    c = Peek();
    if (c != '\0') {
      if (IsTrail(c)) {
        next = next | Mast<6>(Append(Advance())) << 12;
        c = Peek();
        if (c != '\0') {
          if (IsTrail(c)) {
            next = next | Mask<6>(Append(Advance())) << 6;
            c = Peek();
            if (c != '\0') {
              if (IsTrail(c)) {
                next = next | Mask<6>(Append(Advance()));
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
}
