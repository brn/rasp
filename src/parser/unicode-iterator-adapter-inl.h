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
UnicodeIteratorAdapter<16, InputIterator>::UnicodeIteratorAdapter(InputIterator begin)
    : current_position_(0),
      line_number_(1),
      begin_(begin) {}


template <typename InputIterator>
template <typename T>
UnicodeIteratorAdapter<16, InputIterator>::UnicodeIteratorAdapter(const UnicodeIteratorAdapter<16, T>& it)
    : current_position_(it.current_position_),
      line_number_(it.line_number_),
      begin_(it.begin_) {}


template <typename InputIterator>
template <typename T>
UnicodeIteratorAdapter<16, InputIterator>::UnicodeIteratorAdapter(UnicodeIteratorAdapter<16, T>&& it)
    : current_position_(it.current_position_),
      line_number_(it.line_number_),
      begin_(std::move(it.begin_)) {}


template <typename InputIterator>
UChar UnicodeIteratorAdapter<16, InputIterator>::Next () const {
  size_t byte_count = Utf8::GetByteCount(*begin_);
  uint32_t next = ConvertUtf8ToUcs2(byte_count);
  if (next != 0) {
    return UChar(next);
  }
  return UChar();
}


template <typename InputIterator>
uint32_t UnicodeIteratorAdapter<16, InputIterator>::ConvertUtf8ToUcs2(size_t byte_count) const {
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
uint32_t UnicodeIteratorAdapter<16, InputIterator>::Convert2Byte() const {
  const uint8_t kMinimumRange = 0x00000080;
  char c = *begin_;
  if (Utf8::IsNotNull(c)) {
    uint32_t next = Mask<5>(c) << 6;
    c = *(begin_ + 1);
    if (Utf8::IsValidSequence(c)) {
      next = next | Mask<6>(c);
      if (next > kMinimumRange) {
        return next;
      }
    }
  }
  return 0;
}


template <typename InputIterator>
uint32_t UnicodeIteratorAdapter<16, InputIterator>::Convert3Byte() const {
  const int kMinimumRange = 0x00000800;
  char c = *begin_;
  if (Utf8::IsNotNull(c)) {
    uint32_t next = Mask<4>(c) << 12;
    c = *(begin_ + 1);
    if (Utf8::IsValidSequence(c)) {
      next = next | Mask<6>(c) << 6;
      c = *(begin_ + 2);
      if (Utf8::IsValidSequence(c)) {
        next = next | Mask<6>(c);
        if (next > kMinimumRange && Utf16::IsOutOfSurrogateRange(next)) {
          return next;
        }
      }
    }
  }
  return 0;
}


template <typename InputIterator>
uint32_t UnicodeIteratorAdapter<16, InputIterator>::Convert4Byte() const {
  const int kMinimumRange = 0x000010000;
  char c = *begin_;
  if (Utf8::IsNotNull(c)) {
    uint32_t next = Mask<3>(c) << 18;
    c = *(begin_ + 1);
    if (Utf8::IsValidSequence(c)) {
      next = next | Mask<6>(c) << 12;
      c = *(begin_ + 2);
      if (Utf8::IsValidSequence(c)) {
        next = next | Mask<6>(c) << 6;
        c = *(begin_ + 3);
        if (Utf8::IsValidSequence(c)) {
          next = next | Mask<6>(c);
          if (next >= kMinimumRange && next <= 0x10FFFF) {
            return next;
          }
        }
      }
    }
  }
  return 0;
}
}

#endif
