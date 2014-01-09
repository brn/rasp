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
#include "../../src/parser/unicode-iterator-adapter.h"
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


::testing::AssertionResult CompareUchar(const std::string& value, const std::string expected, int i) {
  char a = value.at(i);
  char b = expected.at(i);
  if (a == b) {
    return ::testing::AssertionSuccess();
  }
  return ::testing::AssertionFailure()
      << "The value of index " << i << " expected " << b << " but got " << a
      << "\nvalue:    " << value.substr(0, i) << '\n'
      << "           " << std::string(i - 2, '-') << "^"
      << "\nexpected: " << expected.substr(0, i) << '\n'
      << "           " << std::string(i - 2, '-') << "^"
      << '\n';
}


::testing::AssertionResult IsValid(const rasp::UChar& uchar, const rasp::UnicodeIteratorAdapter<16, std::string::iterator>& un) {
  if (!uchar.IsInvalid()) {
    return ::testing::AssertionSuccess();
  }
  return ::testing::AssertionFailure() << "Invalid unicode charactor. at: " << un.current_position();
}


::testing::AssertionResult IsSurrogatePair(const rasp::UChar& uchar, const rasp::UnicodeIteratorAdapter<16, std::string::iterator>& un) {
  if (uchar.IsSurrogatePair()) {
    return ::testing::AssertionSuccess();
  }
  return ::testing::AssertionFailure() << "Invalid unicode charactor. at: " << un.current_position();
}


void CompareBuffer(const std::string& buffer, const std::string& expectation) {
  for (int i = 0, len = buffer.size(); i < len; i++) {
    ASSERT_TRUE(CompareUchar(buffer, expectation, i));
    i++;
  }
}


TEST(UnicodeIteratorAdapter16, parse_valid_utf8_code_test) {
  std::string source = ReadFile("test/parser/unicode-test-cases/valid-utf8.txt");
  std::string result = ReadFile("test/parser/unicode-test-cases/valid-utf8.result.txt");
  std::string buffer;
  std::vector<rasp::UChar> uchar_vector;
  auto end = source.end();
  rasp::UnicodeIteratorAdapter<16, std::string::iterator> un(source.begin());
  for (;un != end; std::advance(un, 1)) {
    const rasp::UChar uc = *un;
    ASSERT_TRUE(IsValid(uc, un));
    ASSERT_FALSE(IsSurrogatePair(uc, un));
    if (uc.IsAscii()) {
      buffer.append(1, uc.ascii());
    } else {
      rasp::SPrintf(buffer, true, "%#019x", uc.uchar());
    }
    uchar_vector.push_back(uc);
  }
  ASSERT_EQ(uchar_vector.size(), 146);
  CompareBuffer(buffer, result);
}
