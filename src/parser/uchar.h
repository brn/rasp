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

#include <string>
#include "../utils/inline.h"
#include "../utils/os.h"

class UChar {
 public:  
  UChar(uint16_t c, const char* raw, size_t size):
      byte_length_(size),
      uchar_(c) {
    ascii_ = size == 1;
    Strcpy(raw_, raw, size);
  };

  
  UChar()
      : uchar_(0),
        byte_length_(0),
        ascii_(false){}
  
  
  UChar(const UChar& uchar)
      : uchar_(uchar.uchar_),
        byte_length_(uchar.byte_length_),
        ascii_(uchar.ascii_) {
    Strcpy(raw_, uchar.raw_, uchar.byte_length_);
  }
  

  ~UChar() = default;


  INLINE const char* raw() const {return raw_.c_str();}


  INLINE char ascii_char() const {return raw[0];}

  
  INLINE bool invalid() const {
    return uchar_ == 0;
  }


  INLINE bool ascii() const {return ascii_;}

 private:
  size_t byte_length_;
  bool ascii_;
  uint16_t uchar_;
  char[5] raw_;
};
