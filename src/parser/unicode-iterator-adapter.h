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
#include <iterator>
#include <utility>
#include "../utils/inline.h"
#include "../utils/bitmask.h"
#include "../utils/unicode.h"
#include "./uchar.h"

namespace rasp {
template <int Code, typename InputIterator>
class UnicodeIteratorAdapter{};


template <typename InputIterator>
class UnicodeIteratorAdapter<16, InputIterator> : public std::iterator<std::forward_iterator_tag, UChar> {
 public:
  
  
  UnicodeIteratorAdapter<16, InputIterator>(InputIterator begin);

  
  template <typename T>
  UnicodeIteratorAdapter<16, InputIterator>(const UnicodeIteratorAdapter<16, T>& un);


  template <typename T>
  UnicodeIteratorAdapter<16, InputIterator>(UnicodeIteratorAdapter<16, T>&& un);

  
  ~UnicodeIteratorAdapter<16, InputIterator>() = default;


  INLINE UnicodeIteratorAdapter<16, InputIterator>& operator = (InputIterator iter) {begin_ = iter;}
  

  INLINE bool operator == (const InputIterator& iter){return begin_ == iter;}

  
  INLINE bool operator != (const InputIterator& iter){return begin_ != iter;}

  
  INLINE UChar operator* () const {return Next();}


  INLINE UnicodeIteratorAdapter<16, InputIterator>& operator ++() {Advance();return *this;}


  INLINE UnicodeIteratorAdapter<16, InputIterator>& operator +=(int c) {
    while (c--) Advance();
    return *this;
  }


  INLINE const UnicodeIteratorAdapter<16, InputIterator> operator + (int c) {
    UnicodeIteratorAdapter ua(*this);
    while (c--) ++ua;
    return ua;
  }
  
  
  INLINE UC32 line_number() const {return line_number_;}

  
  INLINE UC32 current_position() const {return current_position_;}

  
  INLINE const InputIterator& base() const {return begin_;}

  
 private:

  UChar Next() const;

  
  UC32 ConvertUtf8ToUcs2(size_t byte_count) const;


  INLINE void UnicodeIteratorAdapter::Advance() {
    UC8 next = *begin_;
    size_t byte_count = Utf8::GetByteCount(next);
    if (next == '\n') {
      current_position_ = 1;
      line_number_++;
    } else {
      current_position_ += byte_count;
    }
    std::advance(begin_, byte_count);
  }

  
  template<std::size_t N, typename CharT>
  INLINE CharT Mask(CharT ch) const {
    return Bitmask<N, UC32>::lower & ch;
  }

  
  INLINE UC32 ConvertAscii() const {
    return Mask<8>(*begin_);
  }
  
  
  UC32 Convert2Byte() const;

  
  UC32 Convert3Byte() const;

  
  UC32 Convert4Byte() const;

  
  UC32 current_position_;
  UC32 line_number_;
  InputIterator begin_;
};
}

#include "./unicode-iterator-adapter-inl.h"

#endif
