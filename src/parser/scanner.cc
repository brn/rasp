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
#include "../utils/inline.h"


namespace rasp {


Scanner::Scanner(Source* source)
    : source_(source) {}


Token Scanner::Scan() {
  char peek = source_->Peek();

  if (IsStringLiteralStart(peek)) {
    return std::move(ScanStringLiteral());
  } else if (IsDigitStart(peek)) {
    return std::move(ScanDigit());
  }
  return std::move(Token::Illegal(0, 0, 0, "Unrecognized token."));
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

  return std::move(Token(Token::Type::JS_STRING_LITERAL, 0, 0, 0, ss.str()));
}

Token Scanner::ScanDigit() {
  char ch = source_->Advance();
  char peek = source_->Peek();
  if (ch == '.' && IsNumericLiteral(peek)) {
    return ScanDouble(ch, peek);
  } else if (ch == '0' && peek == 'x') {
    return ScanHex(ch, peek);
  } else if (IsNumericLiteral(ch)) {
    return ScanInteger(ch, peek);
  }
  return std::move(Token::Illegal(0, 0, 0, "Unrecognized token."));
}


Token Scanner::ScanDouble(char ch, char peek) {
  std::stringstream ss;
  ss << ch;
  ch = source_->Advance();
  while (IsNumericLiteral(ch)) {
    ss << ch;
    ch = source_->Advance();
  }
  return std::move(Token(Token::Type::JS_NUMERIC_LITERAL, 0, 0, 0, ss.str()));
}


Token Scanner::ScanHex(char ch, char peek) {
  std::stringstream ss;
  ss << ch;
  ch = source_->Advance();
  ss << ch;
  ch = source_->Advance();
  while (IsHexRange(ch)) {
    ss << ch;
    ch = source_->Advance();
  }
  return std::move(Token(Token::Type::JS_NUMERIC_LITERAL, 0, 0, 0, ss.str()));
}


Token Scanner::ScanInteger(char ch, char peek) {
  std::stringstream ss;
  ss << ch;
  ch = source_->Advance();
  bool js_double = false;
  while (1) {
    if (IsNumericLiteral(ch)) {
      ss << ch;
    } else if (ch == '.' && !js_double && IsNumericLiteral(source_->Peek())) {
      ss << ch;
      ss << source_->Advance();
      js_double = true;
    } else if (ch == '.' && js_double) {
      return std::move(Token::Illegal(0, 0, 0, "Unrecognized token."));
    } else {
      break;
    }
    ch = source_->Advance();
  }
  return std::move(Token(Token::Type::JS_NUMERIC_LITERAL, 0, 0, 0, ss.str()));
}


Token Scanner::ScanIdentifier() {
  return Token(Token::Type::JS_IDENTIFIER, 0, 0, 0, std::string("fooBarBaz"));
}


}
