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
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "test string");
  END_SCAN;
}


TEST(ScannerTest, ScanStringLiteralTest_escaped_string) {
  INIT(token, "'test \\'string'")
  ASSERT_EQ(token.type(), rasp::Token::JS_STRING_LITERAL);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "test \\'string");
  END_SCAN;
}


TEST(ScannerTest, ScanStringLiteralTest_double_escaped_string) {
  INIT(token, "'test \\\\'string'")
  ASSERT_EQ(token.type(), rasp::Token::JS_STRING_LITERAL);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "test \\\\");
}


TEST(ScannerTest, ScanStringLiteralTest_unterminated_string) {
  INIT(token, "'test")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
  ASSERT_STREQ(scanner.message(), "Unterminated string literal.");
}


TEST(ScannerTest, ScanStringLiteralTest_unicode_escaped_string) {
  INIT(token, "'\\u0061_foo_\\u0062_bar_\\u0063_baz'")
  ASSERT_EQ(token.type(), rasp::Token::JS_STRING_LITERAL);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "a_foo_b_bar_c_baz");
}


TEST(ScannerTest, ScanStringLiteralTest_invalid_unicode_escaped_string) {
  INIT(token, "'\\u006_foo_\\u0062_bar_\\u0063_baz'")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
}


TEST(ScannerTest, ScanStringLiteralTest_invalid_unicode_escaped_string2) {
  INIT(token, "'\\u0061_foo_\\u062_bar_\\u0063_baz'")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
}


TEST(ScannerTest, ScanStringLiteralTest_invalid_unicode_escaped_string3) {
  INIT(token, "'\\ux0061_foo_\\u0062_bar_\\u0-063_baz'")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
}


TEST(ScannerTest, ScanDigit_double) {
  INIT(token, ".3032")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), ".3032");
  END_SCAN;
}


TEST(ScannerTest, ScanDigit_double_illegal) {
  INIT(token, ".30.32")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
  ASSERT_STREQ(scanner.message(), "Illegal token.");
}


TEST(ScannerTest, ScanDigit_hex) {
  INIT(token, "0xFFCC33")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "0xFFCC33");
  END_SCAN;
}


TEST(ScannerTest, ScanDigit_int) {
  INIT(token, "1349075")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "1349075");
  END_SCAN;
}


TEST(ScannerTest, ScanDigit_double2) {
  INIT(token, "1349.075")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "1349.075");
  END_SCAN;
}


TEST(ScannerTest, ScanDigit_double2_illegal) {
  INIT(token, "1349.07.5")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
  ASSERT_STREQ(scanner.message(), "Illegal token.");
}


TEST(ScannerTest, ScanDigit_exponent) {
  INIT(token, "1349e+2")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "1349e+2");
  END_SCAN;
}


TEST(ScannerTest, ScanDigit_exponent2) {
  INIT(token, "1.3e+1")
  ASSERT_EQ(token.type(), rasp::Token::JS_NUMERIC_LITERAL);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "1.3e+1");
  END_SCAN;
}


TEST(ScannerTest, ScanDigit_exponent_illegal) {
  INIT(token, "1.3e1")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
  ASSERT_STREQ(scanner.message(), "Illegal token.");
}


TEST(ScannerTest, ScanDigit_exponent_illegal2) {
  INIT(token, "1.3e+")
  ASSERT_EQ(token.type(), rasp::Token::ILLEGAL);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(scanner.message(), "Illegal token.");
}


TEST(ScannerTest, ScanIdentifier_identifier) {
  INIT(token, "fooBarBaz");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "fooBarBaz");
  END_SCAN;
}


TEST(ScannerTest, ScanIdentifier_identifier2) {
  INIT(token, "$_$_foobar");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "$_$_foobar");
  END_SCAN;
}


TEST(ScannerTest, ScanIdentifier_identifier3) {
  INIT(token, "$_$_foobar333_4");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "$_$_foobar333_4");
  END_SCAN;
}


TEST(ScannerTest, ScanIdentifier_long_long_identifier) {
  const char* id = "Lopadotemachoselachogaleokranioleipsanodrimhypotrimmatosilphioparaomelitokatakechymenokichlepikossyphophattoperisteralektryonoptekephallioki";
  INIT(token, id);
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), id);
  END_SCAN;
}


TEST(ScannerTest, ScanIdentifier_identifier_unicode_escape) {
  INIT(token, "\\u0061\\u0062\\u0063");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "abc");
  END_SCAN;
}


TEST(ScannerTest, ScanIdentifier_identifier_unicode_escape_with_ascii) {
  INIT(token, "\\u0061_foo_\\u0062_bar_\\u0063_baz");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  rasp::Utf8Value utf8 = token.value().ToUtf8Value();
  ASSERT_STREQ(utf8.c_str(), "a_foo_b_bar_c_baz");
  END_SCAN;
}


TEST(ScannerTest, ScanLineTerminator_line_terminator) {
  INIT(token, "aaa;");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  ASSERT_TRUE(scanner.has_line_terminator_before_next());
  END_SCAN;
}


TEST(ScannerTest, ScanLineTerminator_line_break) {
  INIT(token, "aaa\n");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  ASSERT_TRUE(scanner.has_line_terminator_before_next());
  END_SCAN;
}


TEST(ScannerTest, ScanLineTerminator_line_terminator_with_space) {
  INIT(token, "aaa  ;");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  ASSERT_TRUE(scanner.has_line_terminator_before_next());
  END_SCAN;
}


TEST(ScannerTest, ScanLineTerminator_line_break_with_space) {
  INIT(token, "aaa  \n");
  ASSERT_EQ(token.type(), rasp::Token::JS_IDENTIFIER);
  ASSERT_TRUE(scanner.has_line_terminator_before_next());
  END_SCAN;
}

