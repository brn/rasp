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
/**
 * Utf-32 representation class.
 */
class UChar {
 public:
  /**
   * Constructor
   * @param c utf-32 byte.
   * @param utf8 utf-8 byte sequence.
   */
  INLINE explicit UChar(UC32 c, const UC8Bytes& utf8):
      uchar_(c),
      utf8_(utf8){};


  /**
   * Constructor
   * Represent invalid utf-8 sequence.
   */
  INLINE UChar(): uchar_(0) {}
  

  /**
   * Copy constructor.
   */
  INLINE UChar(const UChar& uchar):
      uchar_(uchar.uchar_), utf8_(uchar.utf8_) {}
  

  ~UChar() = default;


  /**
   * Assignment
   */
  INLINE const UChar& operator = (const UChar& uchar) {
    uchar_ = uchar.uchar_;
    utf8_ = uchar.utf8_;
    return *(this);
  }
  

  /**
   * Converters
   */
  INLINE explicit operator bool () const {return !IsInvalid();}
  INLINE explicit operator UC16 () const {return uchar();}
  INLINE explicit operator const char* () const {return utf8();}


  /**
   * Return ascii char.
   * @return ascii char.
   */
  INLINE char ToAscii() const {return static_cast<char>(uchar_);}


  /**
   * Check whether utf-16 byte sequence is surrogate pair or not.
   * @return true(if surrogate pair) false(if not surrogate pair)
   */
  INLINE bool IsSurrogatePair() const {
    return Utf16::IsSurrogatePair(uchar_);
  }
  

  /**
   * Convert utf-16 byte sequence to high surrogate byte.
   * @return utf-16 byte which represent high surrogate byte.
   */
  INLINE UC16 ToHighSurrogate() const {
    return Utf16::ToHighSurrogate(uchar_);
  }


  /**
   * Convert utf-16 byte sequence to low surrogate byte.
   * @return utf-16 byte which represent low surrogate byte.
   */
  INLINE UC16 ToLowSurrogate() const {
    return Utf16::ToLowSurrogate(uchar_);
  }
  

  /**
   * Check whether utf-16 byte sequence is invalid or not.
   * @return true(if invalid) false(if valid)
   */
  INLINE bool IsInvalid() const {
    return uchar_ == 0;
  }


  /**
   * Check whether utf-16 byte sequence is within ascii range or not.
   * @return true(if within ascii range) false(if out of ascii range)
   */
  INLINE bool IsAscii() const {return Utf8::IsAscii(uchar_);}


  /**
   * Return utf-16 byte sequence.
   * @return utf-16 byte
   */
  INLINE UC16 uchar() const {return static_cast<UC16>(uchar_);}


  /**
   * Return utf-8 char array that represent this utf-16 byte sequence.
   * @return utf-8 char array.
   */
  INLINE const char* utf8() const {return utf8_.data();}

 private:
  
  UC32 uchar_;
  UC8Bytes utf8_;
};
}


#endif
