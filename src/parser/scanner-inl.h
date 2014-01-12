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

#ifndef PARSER_SCANNER_INL_H_
#define PARSER_SCANNER_INL_H_

#include <cstdio>
#include <sstream>
#include "scanner.h"
#include "token.h"
#include "../utils/inline.h"


namespace rasp {
template<typename InputSourceIterator>
Scanner<InputSourceIterator>::Scanner(InputSourceIterator it, InputSourceIterator end)
    : has_line_terminator_before_next_(false),
      current_position_(0),
      line_number_(1),
      it_(it),
      end_(end){}


template<typename InputSourceIterator>
void Scanner<InputSourceIterator>::Advance()  {
  if (it_ == end_) {
    char_ = UChar::Null();
    return;
  }

  char_ = *it_;
  current_position_++;
  ++it_;
  
  if (char_ == unicode::u8('\n')) {
    line_number_++;
    current_position_ = 1;
  }
  
  if (it_ == end_) {
    lookahead1_ = UChar::Null();
    return;
  }
  lookahead1_ = *it_;
}


template<typename InputSourceIterator>
const TokenInfo& Scanner<InputSourceIterator>::Scan() {
  Advance();
  
  if (char_ == unicode::u8('\0')) {
    BuildToken(Token::END_OF_INPUT);
  } else if (char_ == unicode::u8(';')) {
    return Scan();
  } else if (IsIdentifierStart() || IsUnicodeEscapeSequenceStart()) {
    ScanIdentifier();
  } else if (IsStringLiteralStart()) {
    ScanStringLiteral();
  } else if (IsDigitStart()) {
    ScanDigit();
  } else if (IsOperatorStart()) {
    ScanOperator();
  } else {
    Illegal();
  }
  SkipWhiteSpace();
  return token_info_;
}


template<typename InputSourceIterator>
UC16 Scanner<InputSourceIterator>::ScanHexEscape(const UChar& uchar, int len, bool* success) {
  UC16 result = 0;
  for (int i = 0; i < len; ++i) {
    const int d = ToHexValue(uchar);
    if (d < 0) {
      *success = false;
      return d;
    }
    *success = true;
    result = result * 16 + d;
    Advance();
  }
  return result;
}


template<typename InputSourceIterator>
void Scanner<InputSourceIterator>::ScanStringLiteral() {
  UChar quote = char_;
  UC32Vector<UChar> v;
  bool escaped = false;
  while (1) {
    Advance();
    if (char_ == quote) {
      if (!escaped) {
        break;
      }
      escaped = false;
    } else if (char_ == unicode::u8('\0')) {
      return Error("Unterminated string literal.");
    } else if (!escaped && char_ == unicode::u8('\\') && lookahead1_ == unicode::u8('u')) {
      if (!ScanUnicodeEscapeSequence(&v)) {
        return;
      }
    } else if (char_ == unicode::u8('\\')) {
      escaped = !escaped;
    }
    v += char_;
  }

  BuildToken(Token::JS_STRING_LITERAL, std::move(v));
}


template<typename InputSourceIterator>
void Scanner<InputSourceIterator>::ScanDigit() {
  if (char_ == unicode::u8('0') && lookahead1_ == unicode::u8('x')) {
    return ScanHex();
  }
  
  if ((char_ == unicode::u8('.') && IsNumericLiteral(lookahead1_)) ||
             IsNumericLiteral(char_)) {
    return ScanInteger();
  }
  Illegal();
}


template<typename InputSourceIterator>
void Scanner<InputSourceIterator>::ScanHex() {
  UC32Vector<UChar> v;
  v += char_;
  Advance();
  v += char_;
  Advance();
  while (IsHexRange()) {
    v += char_;
    Advance();
  }
  BuildToken(Token::JS_NUMERIC_LITERAL, std::move(v));
}


template<typename InputSourceIterator>
void Scanner<InputSourceIterator>::ScanInteger() {
  UC32Vector<UChar> v;
  v += char_;
  bool js_double = char_ == unicode::u8('.');
  bool exponent = false;
  bool exponent_operator = false;
  Advance();
  
  while (1) {
    if (IsNumericLiteral(char_)) {
      v += char_;
      if (exponent && !exponent_operator) {
        return Illegal();
      }
      exponent = false;
      exponent_operator = false;
    } else if (exponent && (char_ == unicode::u8('+') || char_ == unicode::u8('-'))) {
      v += char_;
      exponent_operator = true;
    } else if (exponent) {
      return Illegal();
    } else if (!exponent &&
               char_ == unicode::u8('.') &&
               !js_double && IsNumericLiteral(lookahead1_)) {
      v += char_;
      Advance();
      v += char_;
      js_double = true;
    } else if (char_ == unicode::u8('.') && js_double) {
      return Illegal();
    } else if (char_ == unicode::u8('e') || char_ == unicode::u8('E')) {
      exponent = true;
      v += char_;
    } else {
      break;
    }
    Advance();
  }
  if (exponent || exponent_operator) {
    return Illegal();
  }
  
  BuildToken(Token::JS_NUMERIC_LITERAL, std::move(v));
}


template<typename InputSourceIterator>
void Scanner<InputSourceIterator>::ScanIdentifier() {
  UC32Vector<UChar> v;  
  while (IsInIdentifierRange() || char_ == unicode::u8('\\')) {
    if (char_ == unicode::u8('\\')) {
      if (!ScanUnicodeEscapeSequence(&v)) {
        return;
      }
    } else {
      v += char_;
      Advance();
    }
  }
  std::string str = v.ToUtf8Value();
  BuildToken(TokenInfo::GetIdentifierType(str.c_str()), std::move(v));
}


template<typename InputSourceIterator>
void Scanner<InputSourceIterator>::ScanOperator() {
  switch (char_.ToAscii()) {
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
      if (lookahead1_ == unicode::u8('=')) {
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
      Illegal();
  }
}


template<typename InputSourceIterator>
void Scanner<InputSourceIterator>::ScanArithmeticOperator(Token type1, Token type2, Token normal, bool has_let) {
  if (has_let && lookahead1_ == char_) {
    return BuildToken(type1);
  }
  if (lookahead1_ == unicode::u8('=')) {
    return BuildToken(type2);
  }
  BuildToken(normal);
}


template<typename InputSourceIterator>
void Scanner<InputSourceIterator>::ScanBitwiseOrComparationOperator(
    Token type1, Token type2, Token type3, Token normal, Token equal_comparator, bool has_u) {
  if (lookahead1_ == char_) {
    Advance();
    if (lookahead1_ == unicode::u8('=')) {
      Advance();
      return BuildToken(type2);
    } else if (has_u && lookahead1_ == char_) {
      return BuildToken(type3);
    }
    return BuildToken(type1);
  } else if (lookahead1_ == unicode::u8('=')) {
    return BuildToken(equal_comparator);
  }
  BuildToken(normal);
}


template<typename InputSourceIterator>
void Scanner<InputSourceIterator>::ScanEqualityComparator(bool not) {
  if (lookahead1_ == char_) {
    Advance();
    if (!not && lookahead1_ == char_) {
      return BuildToken(Token::JS_EQ);
    }
    return BuildToken(not? Token::JS_NOT_EQ: Token::JS_EQUAL);
  }
  BuildToken(not? Token::JS_NOT_EQUAL: Token::JS_ASSIGN);
}


template<typename InputSourceIterator>
bool Scanner<InputSourceIterator>::ScanUnicodeEscapeSequence(UC32Vector<UChar>* v) {
  Advance();
  if (char_ != unicode::u8('u')) {
    Error("Illegal Token");
    return false;
  }
  Advance();
  bool success;
  UC16 uc16 = ScanHexEscape(char_, 4, &success);
  if (!success || uc16 == '\\' || !IsIdentifierStartChar(uc16)) {
    Illegal();
    return false;
  }
  UC8Bytes bytes = utf16::Convertor::Convert(uc16, 0);
  if (bytes.size() == 0) {
    Illegal();
    return false;
  }
  (*v) += UChar(uc16, bytes);
  return true;
}

} //namespace rasp

#endif
