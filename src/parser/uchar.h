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

#ifndef PARSER_UCHAR_H_
#define PARSER_UCHAR_H_

#include <string>
#include "../utils/inline.h"
#include "../utils/bytelen.h"
#include "../utils/os.h"
#include "../utils/unicode.h"

namespace rasp {
class UChar {
 public:
  INLINE explicit UChar(UC32 c, const UC8Bytes& utf8):
      uchar_(c),
      utf8_(utf8){};

  
  INLINE UChar(): uchar_(0) {}
  
  
  INLINE UChar(const UChar& uchar):
      uchar_(uchar.uchar_), utf8_(uchar.utf8_) {}
  

  ~UChar() = default;

  INLINE operator UC32 () const {return uchar_;}


  INLINE char ascii() const {return static_cast<char>(uchar_);}


  INLINE bool IsSurrogatePair() const {
    return Utf16::IsSurrogatePair(uchar_);
  }
  

  INLINE UC16 high_surrogate() const {
    return Utf16::ToHighSurrogate(uchar_);
  }


  INLINE UC16 low_surrogate() const {
    return Utf16::ToLowSurrogate(uchar_);
  }
  
  
  INLINE bool IsInvalid() const {
    return uchar_ == 0;
  }


  INLINE bool IsAscii() const {return Utf8::IsAscii(uchar_);}


  INLINE UC16 uchar() const {return static_cast<UC16>(uchar_);}


  INLINE const char* utf8() const {return utf8_.data();}

 private:
  
  UC32 uchar_;
  UC8Bytes utf8_;
};
}


#endif
