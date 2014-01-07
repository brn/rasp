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

#include <gtest/gtest.h>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include "../../src/parser/unicode.h"
#include "../../src/parser/uchar.h"


inline std::string ReadFile(const char* filename) {
  std::string tmp_buffer;
  std::stringstream ss;
  std::ifstream file(filename, std::ios::binary | std::ios::in);
  if (file.is_open()) {
    while (std::getline(file, tmp_buffer)) {
      ss << tmp_buffer;
    }
    file.close();
    return ss.str();
  } else {
    throw new std::runtime_error("bad file path.");
  }
}


TEST(Unicode, Unicode_parse_test) {
  std::string source = ReadFile("test/parser/unicode-test-cases/test1.txt");
  rasp::Unicode un(source.c_str());
  std::vector<rasp::UChar> uchar_vector;
  while (un.HasMore()) {
    rasp::UChar uc = un.Next();
    ASSERT_TRUE(!uc.invalid());
    uchar_vector.push_back(uc);
    printf("%#018x\n", uc.uchar());
  }
  printf("%d\n", uchar_vector.size());
}
