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

#ifndef PARSER_TOKEN_INL_H_
#define PARSER_TOKEN_INL_H_

#include "../utils/inline.h";

namespace rasp {
Token::Token(Token token,
    int start_col,
    int end_col,
    int line_number,
    const char* value = nullptr) :
    token_(token),
        value_(value),
        start_col_(start_col),
        end_col_(end_col),
        line_number_(line_number) {
}

Token::Token(const Token& token) {
  this->value_ = token.value_;
  this->token_ = token.token_;
}

INLINE const char* Token::value() const {
  return this->value_;
}

INLINE Token::Type Token::type() const {
  return this->type_;
}

INLINE int Token::start_col() const {
  return this->start_col_;
}

INLINE int Token::end_col() const {
  return this->end_col_;
}

INLINE int Token::line_number() const {
  return this->line_number_;
}
}

#endif
