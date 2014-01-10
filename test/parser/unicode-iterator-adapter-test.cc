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


::testing::AssertionResult IsValid(const rasp::UChar& uchar, const rasp::UnicodeIteratorAdapter<std::string::iterator>& un) {
  if (!uchar.IsInvalid()) {
    return ::testing::AssertionSuccess();
  }
  return ::testing::AssertionFailure() << "Invalid unicode charactor. at: " << un.current_position()
                                       << " value: " + uchar.uchar();
}


::testing::AssertionResult IsSurrogatePair(const rasp::UChar& uchar, const rasp::UnicodeIteratorAdapter<std::string::iterator>& un) {
  if (uchar.IsSurrogatePair()) {
    return ::testing::AssertionSuccess();
  }
  return ::testing::AssertionFailure() << "Invalid unicode charactor. at: " << un.current_position()
                                       << " value: " << uchar.uchar();
}


::testing::AssertionResult IsNotSurrogatePair(const rasp::UChar& uchar, const rasp::UnicodeIteratorAdapter<std::string::iterator>& un) {
  if (uchar.IsSurrogatePair()) {
    return ::testing::AssertionFailure() << "Invalid unicode charactor. at: " << un.current_position()
                                         << " value: " << uchar.uchar();
  }
  return ::testing::AssertionSuccess();
}


void CompareBuffer(const std::string& buffer, const std::string& expectation) {
  for (int i = 0, len = buffer.size(); i < len; i++) {
    ASSERT_TRUE(CompareUchar(buffer, expectation, i));
    i++;
  }
}


inline void UnicodeTest(const char* input, const char* expected, int surrogate_begin, size_t expected_size) {
  std::string source = ReadFile(input);
  std::string result = ReadFile(expected);
  std::string buffer;
  std::string utf8_buffer;
  static const char* kFormat = "%#019x";
  auto end = source.end();
  rasp::UnicodeIteratorAdapter<std::string::iterator> un(source.begin());
  int index = 0;
  int size = 0;
  for (;un != end; std::advance(un, 1)) {
    const rasp::UChar uc = *un;
    ASSERT_TRUE(IsValid(uc, un));
    if (surrogate_begin == -1 || surrogate_begin > index) {
      ASSERT_TRUE(IsNotSurrogatePair(uc, un));
    } else {
      ASSERT_TRUE(IsSurrogatePair(uc, un));
    }
    index++;
    if (uc.IsAscii()) {
      buffer.append(1, uc.ascii());
    } else {
      if (!uc.IsSurrogatePair()) {
        rasp::SPrintf(buffer, true, kFormat, uc.uchar());
      } else {
        rasp::SPrintf(buffer, true, kFormat, uc.high_surrogate());
        rasp::SPrintf(buffer, true, kFormat, uc.low_surrogate());
      }
    }
    utf8_buffer.append(uc.utf8());
    size += uc.IsSurrogatePair()? 2: 1;
  }
  ASSERT_EQ(expected_size, size);
  CompareBuffer(buffer, result);
  CompareBuffer(utf8_buffer, source);
}


TEST(UnicodeIteratorAdapter, parse_valid_utf8_code_test) {
  UnicodeTest("test/parser/unicode-test-cases/valid-utf8.txt",
              "test/parser/unicode-test-cases/valid-utf8.result.txt",
              -1,
              146);
}


TEST(UnicodeIteratorAdapter, parse_valid_utf8_surrogate_pair_code_test) {
  UnicodeTest("test/parser/unicode-test-cases/valid-utf8-surrogate-pair.txt",
              "test/parser/unicode-test-cases/valid-utf8-surrogate-pair.result.txt",
              68,
              674);
}
