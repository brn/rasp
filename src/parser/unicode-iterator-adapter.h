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

#ifndef PARSER_UNICODE_ITERATOR_ADAPTER_H_
#define PARSER_UNICODE_ITERATOR_ADAPTER_H_

#include <array>
#include <stdint.h>
#include <iterator>
#include <utility>
#include "../utils/inline.h"
#include "../utils/bitmask.h"
#include "../utils/unicode.h"
#include "./uchar.h"

namespace rasp {
template <typename InputIterator>
class UnicodeIteratorAdapter : public std::iterator<std::forward_iterator_tag, UChar> {
 public:
  
  
  UnicodeIteratorAdapter(InputIterator begin);

  
  template <typename T>
  UnicodeIteratorAdapter(const UnicodeIteratorAdapter<T>& un);


  template <typename T>
  UnicodeIteratorAdapter(UnicodeIteratorAdapter<T>&& un);

  
  ~UnicodeIteratorAdapter() = default;


  INLINE UnicodeIteratorAdapter& operator = (InputIterator iter) {begin_ = iter;}
  

  INLINE bool operator == (const InputIterator& iter){return begin_ == iter;}

  
  INLINE bool operator != (const InputIterator& iter){return begin_ != iter;}

  
  INLINE UChar operator* () const {return Next();}


  INLINE UnicodeIteratorAdapter& operator ++() {Advance();return *this;}


  INLINE UnicodeIteratorAdapter& operator +=(int c) {
    while (c--) Advance();
    return *this;
  }


  INLINE const UnicodeIteratorAdapter operator + (int c) {
    UnicodeIteratorAdapter ua(*this);
    while (c--) ++ua;
    return ua;
  }
  
  
  INLINE UC32 line_number() const {return line_number_;}

  
  INLINE UC32 current_position() const {return current_position_;}

  
  INLINE const InputIterator& base() const {return begin_;}

  
 private:

  UChar Next() const;

  
  UC32 ConvertUtf8ToUcs2(size_t byte_count, UC8Bytes* utf8) const;


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

  
  INLINE UC32 ConvertAscii(UC8Bytes* utf8) const {
    UC8 uc = *begin_;
    (*utf8)[0] = uc;
    (*utf8)[1] = '\0';
    return Mask<8>(uc);
  }
  
  
  UC32 Convert2Byte(UC8Bytes* utf8) const;

  
  UC32 Convert3Byte(UC8Bytes* utf8) const;

  
  UC32 Convert4Byte(UC8Bytes* utf8) const;

  
  UC32 current_position_;
  UC32 line_number_;
  InputIterator begin_;
};
}

#include "./unicode-iterator-adapter-inl.h"

#endif
