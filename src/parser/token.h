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

#ifndef PARSER_TOKEN_H_
#define PARSER_TOKEN_H_

#include <string>
#include "../utils/inline.h"

namespace rasp {
class Token {
  public:
    enum class Type: int {
      JS_ABSTRACT = 0,
      JS_BOOLEAN,
      JS_BREAK,
      JS_BYTE,
      JS_CASE,
      JS_CATCH,
      JS_CHAR,
      JS_CLASS,
      JS_CONST,
      JS_CONTINUE,
      JS_DEBUGGER,
      JS_DEFAULT,
      JS_DELETE,
      JS_DO,
      JS_DOUBLE,
      JS_ELSE,
      JS_ENUM,
      JS_EXPORT,
      JS_EXTENDS,
      JS_FALSE,
      JS_FINAL,
      JS_FINALLY,
      JS_FLOAT,
      JS_FOR,
      JS_FUNCTION,
      JS_GOTO,
      JS_IF,
      JS_IMPLEMENTS,
      JS_IMPORT,
      JS_IN,
      JS_INSTANCEOF,
      JS_INT,
      JS_INTERFACE,
      JS_LET,
      JS_LONG,
      JS_MODULE,
      JS_NAN,
      JS_NATIVE,
      JS_NEW,
      JS_JS_NULL,
      JS_PACKAGE,
      JS_PRIVATE,
      JS_PROTECTED,
      JS_PUBLIC,
      JS_RETURN,
      JS_SHORT,
      JS_STATIC,
      JS_SUPER,
      JS_SWITCH,
      JS_SYNCHRONIZED,
      JS_THIS,
      JS_THROW,
      JS_THROWS,
      JS_TRANSIENT,
      JS_TRUE,
      JS_TRY,
      JS_TYPEOF,
      JS_VAR,
      JS_VOID,
      JS_VOLATILE,
      JS_WHILE,
      JS_WITH,
      JS_EACH,
      JS_YIELD,
      JS_ASSERT,
      JS_INCREMENT,
      JS_DECREMENT,
      JS_EQUAL,
      JS_SHIFT_LEFT,
      JS_SHIFT_RIGHT,
      JS_LESS_EQUAL,
      JS_GREATER_EQUAL,
      JS_EQ,
      JS_NOT_EQUAL,
      JS_NOT_EQ,
      JS_U_SHIFT_RIGHT,
      JS_ADD_LET,
      JS_SUB_LET,
      JS_DIV_LET,
      JS_MOD_LET,
      JS_MUL_LET,
      JS_LOGICAL_AND,
      JS_LOGICAL_OR,
      JS_SHIFT_LEFT_LET,
      JS_SHIFT_RIGHT_LET,
      JS_U_SHIFT_RIGHT_LET,
      JS_NOT_LET,
      JS_AND_LET,
      JS_OR_LET,
      JS_FUNCTION_GLYPH,
      JS_IDENTIFIER,
      JS_NUMERIC_LITERAL,
      JS_STRING_LITERAL,
      JS_REGEXP_LITERAL,
      JS_LINE_BREAK,
      JS_SET,
      JS_GET,
      JS_REST_PARAMETER,
      END_OF_INPUT,
      ILLEGAL
    };
  

  INLINE static Token Illegal(size_t start_col,
                              size_t end_col,
                              size_t line_number,
                              std::string&& message) {
    return std::move(Token(Token::Type::ILLEGAL, start_col, end_col, line_number, std::move(message)));
  }

  Token(Token::Type token,
               size_t start_col,
               size_t end_col,
               size_t line_number,
               std::string&& value) :
      Token(token, start_col, end_col, line_number) {
    value_ = value;
  }

  Token(Token::Type token,
               size_t start_col,
               size_t end_col,
               size_t line_number) :
      type_(token),
      start_col_(start_col_),
      end_col_(end_col_),
      line_number_(line_number) {
  }

  Token(Token&& token) :
      value_(std::move(token.value_)),
      type_(token.type_),
      start_col_(token.start_col_),
      end_col_(token.end_col_),
      line_number_(token.line_number_) {
  }

  ~Token() = default;

  INLINE const char* value() const {
    return value_.c_str();
  }

  INLINE Token::Type type() const {
    return type_;
  }
  
  INLINE size_t start_col() const {
    return start_col_;
  }

  INLINE size_t end_col() const {
    return end_col_;
  }

  INLINE size_t line_number() const {
    return line_number_;
  }

  private:
    std::string value_;
    Token::Type type_;
    size_t start_col_;
    size_t end_col_;
    size_t line_number_;
};
}
#endif
