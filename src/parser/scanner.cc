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

#include <cstdio>
#include <sstream>
#include "scanner.h"
#include "token.h"

namespace rasp {

Scanner::Scanner(Source* source)
:
    source_(source) {
}
;

Token Scanner::Scan() {
  printf("Hello World");
  return Token(Token::Type::JS_ABSTRACT, 0, 0, 0);
}

Token Scanner::ScanStringLiteral() {
  char quote = source_->Advance();
  std::stringstream ss;
  bool escaped = false;
  while (1) {
    char ch = source_->Advance();
    if (ch == quote) {
      if (!escaped) {
        break;
      }
      escaped = false;
    } else if (ch == '\0') {
      return std::move(Token::Illegal(0, 0, 0, "Unterminated string literal."));
    } else if (ch == '\\') {
      escaped = !escaped;
    }
    ss << ch;
  }

  return Token(Token::Type::JS_STRING_LITERAL, 0, 0, 0, ss.str());
}

Token Scanner::ScanDigit() {
  return Token(Token::Type::JS_NUMERIC_LITERAL, 0, 0, 0, std::string("0"));
}

Token Scanner::ScanIdentifier() {
  return Token(Token::Type::JS_IDENTIFIER, 0, 0, 0, std::string("fooBarBaz"));
}
}
