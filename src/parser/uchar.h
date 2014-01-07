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

namespace rasp {
class UChar {
 public:  
  UChar(uint16_t c, const char* raw):
      byte_length_(STRLEN(raw)),
      uchar_(c) {
    ascii_ = byte_length_ == 1;
    strcpy(raw_, raw);
  };

  
  UChar()
      : uchar_(0),
        byte_length_(0),
        ascii_(false){}
  
  
  UChar(const UChar& uchar)
      : uchar_(uchar.uchar_),
        byte_length_(uchar.byte_length_),
        ascii_(uchar.ascii_) {
    strcpy(raw_, uchar.raw_);
  }
  

  ~UChar() = default;


  INLINE const char* raw() const {return raw_;}


  INLINE char ascii_char() const {return raw_[0];}

  
  INLINE bool invalid() const {
    return uchar_ == 0;
  }


  INLINE bool ascii() const {return ascii_;}


  INLINE uint32_t uchar() const {return uchar_;}

 private:
  size_t byte_length_;
  bool ascii_;
  uint16_t uchar_;
  char raw_[5];
};
}


#endif
