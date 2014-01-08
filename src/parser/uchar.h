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
  UChar(uint16_t c, bool ascii):
      uchar_(c),
      ascii_(ascii){};

  
  UChar()
      : uchar_(0),
        ascii_(false){}
  
  
  UChar(const UChar& uchar)
      : uchar_(uchar.uchar_),
        ascii_(uchar.ascii_) {}
  

  ~UChar() = default;


  INLINE char ascii_char() const {return static_cast<char>(uchar_);}

  
  INLINE bool invalid() const {
    return uchar_ == 0;
  }


  INLINE bool ascii() const {return ascii_;}


  INLINE uint32_t uchar() const {return uchar_;}

 private:
  bool ascii_;
  uint16_t uchar_;
};
}


#endif
