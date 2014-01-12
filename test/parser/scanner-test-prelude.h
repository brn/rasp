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

#ifndef TEST_PARSER_SCANNER_TEST_PRELUDE_H_
#define TEST_PARSER_SCANNER_TEST_PRELUDE_H_

#include <gtest/gtest.h>
#include "../../src/parser/scanner.h"
#include "../../src/parser/uchar.h"
#include "../unicode-util.h"


#define INIT(var, str)                                                  \
  typedef std::vector<rasp::UChar>::iterator Iterator;                  \
  std::vector<rasp::UChar> v = rasp::testing::AsciiToUCharVector(str);  \
  rasp::Scanner<Iterator> scanner(v.begin(), v.end());                  \
  auto var = scanner.Scan();


#define END_SCAN ASSERT_EQ(scanner.Scan().type(), rasp::Token::END_OF_INPUT)

#endif
