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


#ifndef PARSER_CHRACTER_H_
#define PARSER_CHRACTER_H_

#include "uchar.h"
#include "../utils/utils.h"
#include "../utils/unicode.h"

namespace rasp {
class Character : private Static {
 public:

  ALWAYS_INLINE static bool IsWhiteSpace(const UChar& uchar) {
    return uchar.IsAscii() &&
        (uchar == unicode::u8(0x09) ||
         uchar == unicode::u8(0x0b) ||
         uchar == unicode::u8(0x0c) ||
         uchar == unicode::u8(0x20) ||
         uchar == unicode::u8(255) ||
         uchar == unicode::u8('\n') ||
         uchar == unicode::u32(0x2028) ||
         uchar == unicode::u32(0x1680) ||
         uchar == unicode::u32(0x180E) ||
         (uchar >= unicode::u32(0x2000) && uchar <= unicode::u32(0x200A)) ||
         uchar == unicode::u32(0x2028) ||
         uchar == unicode::u32(0x2029) ||
         uchar == unicode::u32(0x202F) ||
         uchar == unicode::u32(0x205F) ||
         uchar == unicode::u32(0x3000));
  }


  ALWAYS_INLINE static bool IsOperatorStart(const UChar& uchar) {
    return uchar.IsAscii() &&
        (uchar == unicode::u8('+') || uchar == unicode::u8('-') || uchar == unicode::u8('*') ||
         uchar == unicode::u8('/') || uchar == unicode::u8('<') || uchar == unicode::u8('>') ||
         uchar == unicode::u8('|') || uchar == unicode::u8('&') || uchar == unicode::u8('~') ||
         uchar == unicode::u8('^') || uchar == unicode::u8('%') || uchar == unicode::u8('=') ||
         uchar == unicode::u8('!'));
  }


  ALWAYS_INLINE static bool IsDigitStart(const UChar& uchar, const UChar& lookahead) {
    return uchar.IsAscii() &&
        ((uchar == unicode::u8('.') && lookahead.IsAscii() && IsNumericLiteral(lookahead)) ||
         (uchar == unicode::u8('0') && lookahead.IsAscii() && lookahead == unicode::u8('x')) ||
         IsNumericLiteral(uchar));
  }

  
  ALWAYS_INLINE static bool IsStringLiteralStart(const UChar& uchar) {
    return uchar.IsAscii() && (uchar == unicode::u8('\'') || uchar == unicode::u8('"'));
  }

  
  ALWAYS_INLINE static bool IsHexRange(const UChar& uchar) {
    return uchar.IsAscii() &&
        (IsNumericLiteral(uchar) ||
         (uchar >= unicode::u8('a') && uchar <= unicode::u8('f')) ||
         (uchar >= unicode::u8('A') && uchar <= unicode::u8('F')));
  }


  ALWAYS_INLINE static bool IsIdentifierStart(const UChar& uchar) {
    return uchar.IsAscii() && IsIdentifierStartChar(uchar);
  }


  template <typename T>
  ALWAYS_INLINE static bool IsIdentifierStartChar(T uchar) {
    return (uchar >= unicode::u8('a') && uchar <= unicode::u8('z')) ||
        (uchar >= unicode::u8('A') && uchar <= unicode::u8('Z')) ||
        (uchar == unicode::u8('_') || uchar == unicode::u8('$'));
  }


  ALWAYS_INLINE static bool IsUnicodeEscapeSequenceStart(const UChar& uchar, const UChar& lookahead) {
    return uchar == unicode::u8('\\') && lookahead == unicode::u8('u');
  }


  ALWAYS_INLINE static bool IsInIdentifierRange(const UChar& uchar) {
    return IsIdentifierStart(uchar) || IsNumericLiteral(uchar);
  }

  
  ALWAYS_INLINE static bool IsNumericLiteral(const UChar& uchar) {
    return uchar.IsAscii() && uchar >= unicode::u8('0') && uchar <= unicode::u8('9');
  }


  ALWAYS_INLINE static bool IsBinaryCharacter(const UChar& uchar) {
    return uchar.IsAscii() && (uchar == unicode::u8('0') || uchar == unicode::u8('1'));
  }
};
} //namespace rasp

#endif
