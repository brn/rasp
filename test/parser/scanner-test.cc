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


#include "./scanner-test-prelude.h"


TEST(ScannerTest, ScanStringLiteralTest_normal) {
  INIT(token, "'test string'")
  ASSERT_EQ(token.type(), rasp::Token::JS_STRING_LITERAL);
  ASSERT_STREQ(token.value(), "test string");
}

TEST(ScannerTest, ScanStringLiteralTest_escaped_string) {
  INIT(token, "'test \\'string'")
  ASSERT_EQ(token.type(), rasp::Token::JS_STRING_LITERAL);
  ASSERT_STREQ(token.value(), "test \\'string");
}

TEST(ScannerTest, ScanStringLiteralTest_double_escaped_string) {
  INIT(token, "'test \\\\'string'")
  ASSERT_EQ(token.type(), rasp::Token::JS_STRING_LITERAL);
  ASSERT_STREQ(token.value(), "test \\\\");
}

TEST(ScannerTest, ScanStringLiteralTest_unterminated_string) {
  INIT(token, "'test")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
  ASSERT_STREQ(token.value(), "Unterminated string literal.");
}

TEST(ScannerTest, ScanDigit_double) {
  INIT(token, ".3032")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  ASSERT_STREQ(token.value(), ".3032");
}

TEST(ScannerTest, ScanDigit_double_illegal) {
  INIT(token, ".30.32")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
  ASSERT_STREQ(token.value(), "Illegal token.");
}

TEST(ScannerTest, ScanDigit_hex) {
  INIT(token, "0xFFCC33")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  ASSERT_STREQ(token.value(), "0xFFCC33");
}

TEST(ScannerTest, ScanDigit_int) {
  INIT(token, "1349075")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  ASSERT_STREQ(token.value(), "1349075");
}

TEST(ScannerTest, ScanDigit_double2) {
  INIT(token, "1349.075")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  ASSERT_STREQ(token.value(), "1349.075");
}

TEST(ScannerTest, ScanDigit_double2_illegal) {
  INIT(token, "1349.07.5")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
  ASSERT_STREQ(token.value(), "Illegal token.");
}


TEST(ScannerTest, ScanDigit_exponent) {
  INIT(token, "1349e+2")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  ASSERT_STREQ(token.value(), "1349e+2");
}


TEST(ScannerTest, ScanDigit_exponent2) {
  INIT(token, "1.3e+1")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  ASSERT_STREQ(token.value(), "1.3e+1");
}


TEST(ScannerTest, ScanDigit_exponent_illegal) {
  INIT(token, "1.3e1")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
  ASSERT_STREQ(token.value(), "Illegal token.");
}


TEST(ScannerTest, ScanDigit_exponent_illegal2) {
  INIT(token, "1.3e+")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
  ASSERT_STREQ(token.value(), "Illegal token.");
}


TEST(ScannerTest, ScanIdentifier_identifier) {
  INIT(token, "fooBarBaz");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  ASSERT_STREQ(token.value(), "fooBarBaz");
}


TEST(ScannerTest, ScanIdentifier_identifier2) {
  INIT(token, "$_$_foobar");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  ASSERT_STREQ(token.value(), "$_$_foobar");
}


TEST(ScannerTest, ScanIdentifier_identifier3) {
  INIT(token, "$_$_foobar333_4");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  ASSERT_STREQ(token.value(), "$_$_foobar333_4");
}


TEST(ScannerTest, ScanLineTerminator) {
  INIT(token, ";");
  ASSERT_EQ(token.type(), rasp::Token::LINE_TERMINATOR);
}

