/*
 * The MIT License (MIT)
 * 
 * Copyright (c) 2013 Taketoshi Aono(brn)
 * 
 * Permission is hereby granted, free of Charge, to any person obtaining a copy
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


const TokenInfo& Scanner::Scan() {
  Advance();
  SkipWhiteSpace();
  
  switch(char_) {
    case '\0':
      return BuildToken(Token::END_OF_INPUT);
    case ';':
      return BuildToken(Token::LINE_TERMINATOR);
    default:
      if (IsIdentifierStart()) {
        return ScanIdentifier();
      }

      if (IsStringLiteralStart()) {
        return ScanStringLiteral();
      }

      if (IsDigitStart()) {
        return ScanDigit();
      }

      if (IsOperatorStart()) {
        return ScanOperator();
      }
      return Illegal();
  }
}


const TokenInfo& Scanner::ScanStringLiteral() {
  Char quote = char_;
  std::stringstream ss;
  bool escaped = false;
  while (1) {
    Advance();
    if (char_ == quote) {
      if (!escaped) {
        break;
      }
      escaped = false;
    } else if (char_ == '\0') {
      return Error("Unterminated string literal.");
    } else if (char_ == '\\') {
      escaped = !escaped;
    }
    ss.put(char_);
  }

  return BuildToken(Token::JS_STRING_LITERAL, ss);
}


const TokenInfo& Scanner::ScanDigit() {
  if (char_ == '0' && lookahead1_ == 'x') {
    return ScanHex();
  } else if ((char_ == '.' && IsNumericLiteral(lookahead1_)) ||
             IsNumericLiteral(char_)) {
    return ScanInteger();
  }
  return Illegal();
}


const TokenInfo& Scanner::ScanHex() {
  std::stringstream ss;
  ss.put(char_);
  Advance();
  ss.put(char_);
  Advance();
  while (IsHexRange()) {
    ss.put(char_);
    Advance();
  }
  return BuildToken(Token::JS_NUMERIC_LITERAL, ss);
}


const TokenInfo& Scanner::ScanInteger() {
  std::stringstream ss;
  ss.put(char_);
  bool js_double = char_ == '.';
  bool exponent = false;
  bool exponent_operator = false;
  Advance();
  
  while (1) {
    if (IsNumericLiteral(char_)) {
      ss.put(char_);
      if (exponent && !exponent_operator) {
        return Illegal();
      }
      exponent = false;
      exponent_operator = false;
    } else if (exponent && (char_ == '+' || char_ == '-')) {
      ss.put(char_);
      exponent_operator = true;
    } else if (exponent) {
      return Illegal();
    } else if (!exponent &&
               char_ == '.' &&
               !js_double && IsNumericLiteral(lookahead1_)) {
      ss.put(char_);
      Advance();
      ss.put(char_);
      js_double = true;
    } else if (char_ == '.' && js_double) {
      return Illegal();
    } else if (char_ == 'e' || char_ == 'E') {
      exponent = true;
      ss.put(char_);
    } else {
      break;
    }
    Advance();
  }
  if (exponent || exponent_operator) {
    return Illegal();
  }
  return BuildToken(Token::JS_NUMERIC_LITERAL, ss);
}


const TokenInfo& Scanner::ScanIdentifier() {
  std::stringstream ss;
  ss.put(char_);
  Advance();
  while (IsInIdentifierRange()) {
    ss.put(char_);
    Advance();
  }
  std::string&& str = ss.str();
  return BuildToken(TokenInfo::GetIdentifierType(str.c_str()), str);
}


const TokenInfo& Scanner::ScanOperator() {
  switch (char_) {
    case '+':
      return ScanArithmeticOperator(Token::JS_INCREMENT, Token::JS_ADD_LET, Token::JS_PLUS);
    case '-':
      return ScanArithmeticOperator(Token::JS_DECREMENT, Token::JS_SUB_LET, Token::JS_MINUS);
    case '*':
      return ScanArithmeticOperator(Token::ILLEGAL, Token::JS_MUL_LET, Token::JS_MUL, false);
    case '/':
      return ScanArithmeticOperator(Token::ILLEGAL, Token::JS_DIV_LET, Token::JS_DIV, false);
    case '%':
      return ScanArithmeticOperator(Token::ILLEGAL, Token::JS_MOD_LET, Token::JS_MOD, false);
    case '~':
      return ScanArithmeticOperator(Token::ILLEGAL, Token::JS_NOR_LET, Token::JS_BIT_NOR, false);
    case '^':
      return ScanArithmeticOperator(Token::ILLEGAL, Token::JS_XOR_LET, Token::JS_BIT_XOR, false);
    case '&':
      return ScanLogicalOperator(Token::JS_LOGICAL_AND, Token::JS_AND_LET, Token::JS_BIT_AND);
    case '|':
      return ScanLogicalOperator(Token::JS_LOGICAL_OR, Token::JS_OR_LET, Token::JS_BIT_OR);
    case '=':
      return ScanEqualityComparator();
    case '!':
      if (lookahead1_ == '=') {
        Advance();
        return ScanEqualityComparator(true);
      }
      return BuildToken(Token::JS_NOT);
    case '<':
      return ScanBitwiseOrComparationOperator(
          Token::JS_SHIFT_LEFT, Token::JS_SHIFT_LEFT_LET, Token::ILLEGAL, Token::JS_LESS, Token::JS_LESS_EQUAL);
    case '>':
      return ScanBitwiseOrComparationOperator(
          Token::JS_SHIFT_RIGHT, Token::JS_SHIFT_RIGHT_LET,
          Token::JS_U_SHIFT_RIGHT, Token::JS_GREATER, Token::JS_GREATER_EQUAL, true);
    default:
      return Illegal();
  }
}


const TokenInfo& Scanner::ScanArithmeticOperator(Token type1, Token type2, Token normal, bool has_let) {
  if (has_let && lookahead1_ == char_) {
    return BuildToken(type1);
  }
  if (lookahead1_ == '=') {
    return BuildToken(type2);
  }
  return BuildToken(normal);
}


const TokenInfo& Scanner::ScanBitwiseOrComparationOperator(
    Token type1, Token type2, Token type3, Token normal, Token equal_comparator, bool has_u) {
  if (lookahead1_ == char_) {
    Advance();
    if (lookahead1_ == '=') {
      Advance();
      return BuildToken(type2);
    } else if (has_u && lookahead1_ == char_) {
      return BuildToken(type3);
    }
    return BuildToken(type1);
  } else if (lookahead1_ == '=') {
    return BuildToken(equal_comparator);
  }
  return BuildToken(normal);
}


const TokenInfo& Scanner::ScanEqualityComparator(bool not) {
  if (lookahead1_ == char_) {
    Advance();
    if (!not && lookahead1_ == char_) {
      return BuildToken(Token::JS_EQ);
    }
    return BuildToken(not? Token::JS_NOT_EQ: Token::JS_EQUAL);
  }
  return BuildToken(not? Token::JS_NOT_EQUAL: Token::JS_ASSIGN);
}

}

