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


#include <type_traits>
#include "../utils/utils.h"
#include "token.h"


const char* kTokenStringList[] = {
    "JS_BREAK",
    "JS_CASE",
    "JS_CATCH",
    "JS_CLASS",
    "JS_CONST",
    "JS_CONTINUE",
    "JS_DEBUGGER",
    "JS_DEFAULT",
    "JS_DELETE",
    "JS_DO",
    "JS_ELSE",
    "JS_ENUM",
    "JS_EXPORT",
    "JS_EXTENDS",
    "JS_FALSE",
    "JS_FINALLY",
    "JS_FOR",
    "JS_FUNCTION",
    "JS_IF",
    "JS_IMPLEMENTS",
    "JS_IMPORT",
    "JS_IN",
    "JS_INSTANCEOF",
    "JS_INTERFACE",
    "JS_LET",
    "JS_NAN",
    "JS_NEW",
    "JS_NULL",
    "JS_PACKAGE",
    "JS_PRIVATE",
    "JS_PROTECTED",
    "JS_PUBLIC",
    "JS_RETURN",
    "JS_STATIC",
    "JS_SUPER",
    "JS_SWITCH",
    "JS_THIS",
    "JS_THROW",
    "JS_TRUE",
    "JS_TRY",
    "JS_TYPEOF",
    "JS_UNDEFINED",
    "JS_VAR",
    "JS_VOID",
    "JS_WHILE",
    "JS_WITH",
    "JS_YIELD",
    "JS_INCREMENT",
    "JS_DECREMENT",
    "JS_EQUAL",
    "JS_SHIFT_LEFT",
    "JS_SHIFT_RIGHT",
    "JS_LESS_EQUAL",
    "JS_GREATER_EQUAL",
    "JS_EQ",
    "JS_NOT_EQUAL",
    "JS_NOT_EQ",
    "JS_U_SHIFT_RIGHT",
    "JS_PLUS",
    "JS_MINUS",
    "JS_MUL",
    "JS_DIV",
    "JS_MOD",
    "JS_GREATER",
    "JS_LESS",
    "JS_BIT_OR",
    "JS_BIT_AND",
    "JS_BIT_NOR",
    "JS_BIT_XOR",
    "JS_ASSIGN",
    "JS_NOT",
    "JS_ADD_LET",
    "JS_SUB_LET",
    "JS_DIV_LET",
    "JS_MOD_LET",
    "JS_MUL_LET",
    "JS_LOGICAL_AND",
    "JS_LOGICAL_OR",
    "JS_SHIFT_LEFT_LET",
    "JS_SHIFT_RIGHT_LET",
    "JS_U_SHIFT_RIGHT_LET",
    "JS_NOR_LET",
    "JS_AND_LET",
    "JS_OR_LET",
    "JS_XOR_LET",
    "JS_FUNCTION_GLYPH",
    "JS_IDENTIFIER",
    "JS_NUMERIC_LITERAL",
    "JS_OCTAL_LITERAL",
    "JS_BINARY_LITERAL",
    "JS_STRING_LITERAL",
    "JS_REGEXP_LITERAL",
    "JS_LINE_BREAK",
    "JS_SET",
    "JS_GET",
    "JS_REST_PARAMETER",
    "LINE_TERMINATOR",
    "FUTURE_STRICT_RESERVED_WORD",
    "FUTURE_RESERVED_WORD",
    "END_OF_INPUT",
    "ILLEGAL"
  };



#define KEYWORDS(KEYWORD_GROUP, KEYWORD)                                \
  KEYWORD_GROUP('b')                                                    \
  KEYWORD("break", Token::JS_BREAK)                                     \
  KEYWORD_GROUP('c')                                                    \
  KEYWORD("case", Token::JS_CASE)                                       \
  KEYWORD("catch", Token::JS_CATCH)                                     \
  KEYWORD("class", Token::FUTURE_RESERVED_WORD)                         \
  KEYWORD("const", Token::JS_CONST)                                     \
  KEYWORD("continue", Token::JS_CONTINUE)                               \
  KEYWORD_GROUP('d')                                                    \
  KEYWORD("debugger", Token::JS_DEBUGGER)                               \
  KEYWORD("default", Token::JS_DEFAULT)                                 \
  KEYWORD("delete", Token::JS_DELETE)                                   \
  KEYWORD("do", Token::JS_DO)                                           \
  KEYWORD_GROUP('e')                                                    \
  KEYWORD("else", Token::JS_ELSE)                                       \
  KEYWORD("enum", Token::FUTURE_RESERVED_WORD)                          \
  KEYWORD("export", es_harmony? Token::JS_EXPORT: Token::FUTURE_RESERVED_WORD) \
  KEYWORD("extends", Token::FUTURE_RESERVED_WORD)                       \
  KEYWORD_GROUP('f')                                                    \
  KEYWORD("false", Token::JS_FALSE)                                     \
  KEYWORD("finally", Token::JS_FINALLY)                                 \
  KEYWORD("for", Token::JS_FOR)                                         \
  KEYWORD("function", Token::JS_FUNCTION)                               \
  KEYWORD_GROUP('i')                                                    \
  KEYWORD("if", Token::JS_IF)                                           \
  KEYWORD("implements", Token::FUTURE_STRICT_RESERVED_WORD)             \
  KEYWORD("import", es_harmony? Token::JS_IMPORT: Token::FUTURE_RESERVED_WORD) \
  KEYWORD("in", Token::JS_IN)                                           \
  KEYWORD("instanceof", Token::JS_INSTANCEOF)                           \
  KEYWORD("interface", Token::FUTURE_STRICT_RESERVED_WORD)              \
  KEYWORD_GROUP('l')                                                    \
  KEYWORD("let", es_harmony? Token::JS_LET: Token::FUTURE_STRICT_RESERVED_WORD) \
  KEYWORD_GROUP('n')                                                    \
  KEYWORD("new", Token::JS_NEW)                                         \
  KEYWORD("null", Token::JS_NULL)                                       \
  KEYWORD_GROUP('N')                                                    \
  KEYWORD("NaN", Token::JS_NAN)                                         \
  KEYWORD_GROUP('p')                                                    \
  KEYWORD("package", Token::FUTURE_STRICT_RESERVED_WORD)                \
  KEYWORD("private", Token::FUTURE_STRICT_RESERVED_WORD)                \
  KEYWORD("protected", Token::FUTURE_STRICT_RESERVED_WORD)              \
  KEYWORD("public", Token::FUTURE_STRICT_RESERVED_WORD)                 \
  KEYWORD_GROUP('r')                                                    \
  KEYWORD("return", Token::JS_RETURN)                                   \
  KEYWORD_GROUP('s')                                                    \
  KEYWORD("static", Token::FUTURE_STRICT_RESERVED_WORD)                 \
  KEYWORD("super", Token::FUTURE_RESERVED_WORD)                         \
  KEYWORD("switch", Token::JS_SWITCH)                                   \
  KEYWORD_GROUP('t')                                                    \
  KEYWORD("this", Token::JS_THIS)                                       \
  KEYWORD("throw", Token::JS_THROW)                                     \
  KEYWORD("true", Token::JS_TRUE)                                       \
  KEYWORD("try", Token::JS_TRY)                                         \
  KEYWORD("typeof", Token::JS_TYPEOF)                                   \
  KEYWORD_GROUP('u')                                                    \
  KEYWORD("undefined", Token::JS_UNDEFINED)                             \
  KEYWORD_GROUP('v')                                                    \
  KEYWORD("var", Token::JS_VAR)                                         \
  KEYWORD("void", Token::JS_VOID)                                       \
  KEYWORD_GROUP('w')                                                    \
  KEYWORD("while", Token::JS_WHILE)                                     \
  KEYWORD("with", Token::JS_WITH)                                       \
  KEYWORD_GROUP('y')                                                    \
  KEYWORD("yield", Token::JS_YIELD)


namespace rasp {

const char* TokenInfo::ToString() const {
  return kTokenStringList[static_cast<uint16_t>(type_)];
}

Token TokenInfo::GetIdentifierType(const char* maybe_keyword, bool es_harmony) {
  const int input_length = Strlen(maybe_keyword);
  const int min_length = 2;
  const int max_length = 10;
  if (input_length < min_length || input_length > max_length) {
    return Token::JS_IDENTIFIER;
  }
  
  switch (maybe_keyword[0]) {
    default:
#define KEYWORD_GROUP_CASE(ch)                  \
      break;                                      \
    case ch:
#define KEYWORD(keyword, token)                               \
    {                                                         \
      const int keyword_length = sizeof(keyword) - 1;                   \
      static_assert(keyword_length >= min_length, "The length of the keyword must be greater than 2"); \
      static_assert(keyword_length <= max_length, "The length of the keyword mst be less than 10"); \
      if (input_length == keyword_length &&                           \
          maybe_keyword[1] == keyword[1] &&                           \
          (keyword_length <= 2 || maybe_keyword[2] == keyword[2]) &&  \
          (keyword_length <= 3 || maybe_keyword[3] == keyword[3]) &&  \
          (keyword_length <= 4 || maybe_keyword[4] == keyword[4]) &&  \
          (keyword_length <= 5 || maybe_keyword[5] == keyword[5]) &&  \
          (keyword_length <= 6 || maybe_keyword[6] == keyword[6]) &&  \
          (keyword_length <= 7 || maybe_keyword[7] == keyword[7]) &&  \
          (keyword_length <= 8 || maybe_keyword[8] == keyword[8]) &&  \
          (keyword_length <= 9 || maybe_keyword[9] == keyword[9])) {  \
        return token;                                                 \
      }                                                               \
    }
    KEYWORDS(KEYWORD_GROUP_CASE, KEYWORD)
  }
  return Token::JS_IDENTIFIER;
}
}
