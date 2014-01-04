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
#include "../../src/parser/scanner.h"

#define INIT(var, str)\
  rasp::Source source((str));\
  rasp::Scanner scanner(&source);\
  auto var = scanner.Scan();

TEST(ScannerTest, ScanStringLiteralTest_normal) {
  INIT(token, "'test string'")
  ASSERT_EQ(token.type(), rasp::Token::Type::JS_STRING_LITERAL);
  ASSERT_STREQ(token.value(), "test string");
}

TEST(ScannerTest, ScanStringLiteralTest_escaped_string) {
  INIT(token, "'test \\'string'")
  ASSERT_EQ(token.type(), rasp::Token::Type::JS_STRING_LITERAL);
  ASSERT_STREQ(token.value(), "test \\'string");
}

TEST(ScannerTest, ScanStringLiteralTest_double_escaped_string) {
  INIT(token, "'test \\\\'string'")
  ASSERT_EQ(token.type(), rasp::Token::Type::JS_STRING_LITERAL);
  ASSERT_STREQ(token.value(), "test \\\\");
}

TEST(ScannerTest, ScanStringLiteralTest_unterminated_string) {
  INIT(token, "'test")
  ASSERT_EQ(token.type(), rasp::Token::Type::ILLEGAL);
  ASSERT_STREQ(token.value(), "Unterminated string literal.");
}

TEST(ScannerTest, ScanDigit_double) {
  INIT(token, ".3032")
  ASSERT_EQ(token.type(), rasp::Token::Type::JS_NUMERIC_LITERAL);
  ASSERT_STREQ(token.value(), ".3032");
}

TEST(ScannerTest, ScanDigit_hex) {
  INIT(token, "0xFFCC33")
  ASSERT_EQ(token.type(), rasp::Token::Type::JS_NUMERIC_LITERAL);
  ASSERT_STREQ(token.value(), "0xFFCC33");
}

TEST(ScannerTest, ScanDigit_int) {
  INIT(token, "1349075")
  ASSERT_EQ(token.type(), rasp::Token::Type::JS_NUMERIC_LITERAL);
  ASSERT_STREQ(token.value(), "1349075");
}

TEST(ScannerTest, ScanDigit_double2) {
  INIT(token, "1349.075")
  ASSERT_EQ(token.type(), rasp::Token::Type::JS_NUMERIC_LITERAL);
  ASSERT_STREQ(token.value(), "1349.075");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
