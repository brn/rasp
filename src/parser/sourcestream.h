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

#ifndef PARSER_SOURCESTREAM_H_
#define PARSER_SOURCESTREAM_H_

#include <iterator>
#include <string>
#include "../utils/class-trait.h"
#include "../utils/inline.h"
#include "../utils/unicode.h"
#include "../utils/stat.h"
#include "../utils/os.h"
#include "../utils/error-reporter.h"

namespace rasp {

class SourceStream : public MaybeFail<FileIOException>, private Uncopyable {
 public:
  SourceStream(const char* filepath);

  INLINE ~SourceStream() = default;
  
  
  INLINE std::string::iterator begin() {return buffer_.begin();}
  

  INLINE std::string::iterator end() {return buffer_.end();}


  INLINE const char* buffer() {return buffer_.c_str();}


  INLINE size_t size() const {return size_;}


 private:
  INLINE void ReadBlock(FILE* fp)  {
    char* buffer = new char[size_ + 1];
    size_t next = FRead(buffer, size_, sizeof(UC8), size_ - 1, fp);
    if (next > 0) {
      buffer[next] = '\0';
      buffer_.append(buffer);
    }
    delete[] buffer;
  }

  size_t size_;
  std::string filepath_;
  std::string buffer_;
};

}
#endif
