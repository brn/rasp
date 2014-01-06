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
#include "../utils/bytelen.h"
#include "../utils/inline.h"
#include "../utils/os.h"

#define FAST_VALUE_LENGTH 20

namespace rasp {
enum class Token: int {
  JS_BREAK,
  JS_CASE,
  JS_CATCH,
  JS_CLASS,
  JS_CONST,
  JS_CONTINUE,
  JS_DEBUGGER,
  JS_DEFAULT,
  JS_DELETE,
  JS_DO,
  JS_ELSE,
  JS_ENUM,
  JS_EXPORT,
  JS_EXTENDS,
  JS_FALSE,
  JS_FINALLY,
  JS_FOR,
  JS_FUNCTION,
  JS_IF,
  JS_IMPLEMENTS,
  JS_IMPORT,
  JS_IN,
  JS_INSTANCEOF,
  JS_INTERFACE,
  JS_LET,
  JS_NAN,
  JS_NEW,
  JS_NULL,
  JS_PACKAGE,
  JS_PRIVATE,
  JS_PROTECTED,
  JS_PUBLIC,
  JS_RETURN,
  JS_STATIC,
  JS_SUPER,
  JS_SWITCH,
  JS_THIS,
  JS_THROW,
  JS_TRUE,
  JS_TRY,
  JS_TYPEOF,
  JS_UNDEFINED,
  JS_VAR,
  JS_VOID,
  JS_WHILE,
  JS_WITH,
  JS_YIELD,
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
  JS_PLUS,
  JS_MINUS,
  JS_MUL,
  JS_DIV,
  JS_MOD,
  JS_GREATER,
  JS_LESS,
  JS_BIT_OR,
  JS_BIT_AND,
  JS_BIT_NOR,
  JS_BIT_XOR,
  JS_ASSIGN,
  JS_NOT,
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
  JS_NOR_LET,
  JS_AND_LET,
  JS_OR_LET,
  JS_XOR_LET,
  JS_FUNCTION_GLYPH,
  JS_IDENTIFIER,
  JS_NUMERIC_LITERAL,
  JS_STRING_LITERAL,
  JS_REGEXP_LITERAL,
  JS_LINE_BREAK,
  JS_SET,
  JS_GET,
  JS_REST_PARAMETER,
  LINE_TERMINATOR,
  FUTURE_STRICT_RESERVED_WORD,
  FUTURE_RESERVED_WORD,
  END_OF_INPUT,
  ILLEGAL
};


class TokenInfo {
 public:

  TokenInfo() :
      type_(Token::END_OF_INPUT),
      start_col_(0),
      line_number_(1) {
    short_value_[0] = '\0';
  }
  

  TokenInfo(const Token& token) = delete;

  
  ~TokenInfo() = default;


  INLINE void set_value(const char* value) {
    const size_t max = sizeof(short_value_);
    size_t size = BYTELEN(value);
    if (size < max) {
      Strcpy(short_value_, value, max);
    } else {
      short_value_[0] = '\0';
      value_.assign(value);
    }
  }
  
  
  INLINE const char* value() const {
    return short_value_[0] != '\0'? short_value_ : value_.c_str();
  }


  INLINE void set_type(Token type) {
    type_ = type;
  }
  
  
  INLINE Token type() const {
    return type_;
  }


  INLINE void set_start_col(size_t start_col) {
    start_col_ = start_col;
  }
  
  
  INLINE size_t start_col() const {
    return start_col_;
  }


  INLINE void set_line_number(size_t line_number) {
    line_number_ = line_number;
  }
  
  
  INLINE size_t line_number() const {
    return line_number_;
  }


  INLINE void set_has_line_break_before_next(bool has) {
    has_line_break_before_next_ = has;
  }
  
  
  INLINE bool has_line_break_before_next() const {
    return has_line_break_before_next_;
  }

  static Token GetIdentifierType(const char* maybe_keyword, bool es_harmony = false);
  
 private:
  char short_value_[FAST_VALUE_LENGTH];
  std::string value_;
  Token type_;
  size_t start_col_;
  size_t line_number_;
  bool has_line_break_before_next_;
};
}
#undef FAST_VALUE_LENGTH
#endif
