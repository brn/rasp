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

#include "../utils/inline.h";

namespace rasp {
class Token {
  public:
    enum Type {
      ABSTRACT = 0,
      BOOLEAN,
      BREAK,
      BYTE,
      CASE,
      CATCH,
      CHAR,
      CLASS,
      CONST,
      CONTINUE,
      DEBUGGER,
      DEFAULT,
      DELETE,
      DO,
      DOUBLE,
      ELSE,
      ENUM,
      EXPORT,
      EXTENDS,
      FALSE,
      FINAL,
      FINALLY,
      FLOAT,
      FOR,
      FUNCTION,
      GOTO,
      IF,
      IMPLEMENTS,
      IMPORT,
      IN,
      INSTANCEOF,
      INT,
      INTERFACE,
      LET,
      LONG,
      MODULE,
      NAN,
      NATIVE,
      NEW,
      NULL,
      PACKAGE,
      PRIVATE,
      PROTECTED,
      PUBLIC,
      RETURN,
      SHORT,
      STATIC,
      SUPER,
      SWITCH,
      SYNCHRONIZED,
      THIS,
      THROW,
      THROWS,
      TRANSIENT,
      TRUE,
      TRY,
      TYPEOF,
      VAR,
      VOID,
      VOLATILE,
      WHILE,
      WITH,
      EACH,
      YIELD,
      ASSERT,
      INCREMENT,
      DECREMENT,
      EQUAL,
      SHIFT_LEFT,
      SHIFT_RIGHT,
      LESS_EQUAL,
      GREATER_EQUAL,
      EQ,
      NOT_EQUAL,
      NOT_EQ,
      U_SHIFT_RIGHT,
      ADD_LET,
      SUB_LET,
      DIV_LET,
      MOD_LET,
      MUL_LET,
      LOGICAL_AND,
      LOGICAL_OR,
      SHIFT_LEFT_LET,
      SHIFT_RIGHT_LET,
      U_SHIFT_RIGHT_LET,
      NOT_LET,
      AND_LET,
      OR_LET,
      FUNCTION_GLYPH,
      IDENTIFIER,
      NUMERIC_LITERAL,
      STRING_LITERAL,
      REGEXP_LITERAL,
      LINE_BREAK,
      SET,
      GET,
      REST_PARAMETER,
      EOF
    };

    Token(Token token,
        int start_col,
        int end_col,
        int line_number,
        const char* value = nullptr);

    Token(const Token& token);

    ~Token() = default;

    INLINE
    const char* value() const;

    INLINE
    Token type() const;

    INLINE
    int start_col() const;

    INLINE
    int end_col() const;

    INLINE
    int line_number() const;

  private:
    const char* value_;
    Token::Type token_;
    int start_col_;
    int end_col_;
    int line_number_;
}
}

#include "token-inl.h";
#endif
