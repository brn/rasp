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

#ifndef PARSER_SCANNER_H_
#define PARSER_SCANNER_H_

#include <sstream>
#include "source.h"
#include "token.h"
#include "uc32-vector.h"


namespace rasp {
template <typename InputSourceIterator>
class Scanner {
 public:
  /**
   * @param source The source file content.
   */
  Scanner(InputSourceIterator it, InputSourceIterator end);

  /**
   * Scan the source file from the current position to the next token position.
   */
  const TokenInfo& Scan();


  INLINE bool has_line_terminator_before_next() const {
    return has_line_terminator_before_next_;
  }
  
  
  INLINE const char* message() const {
    return message_.c_str();
  }

 private:  
  /**
   * Scan string literal.
   */
  void ScanStringLiteral();

  /**
   * Scan digit literal(includes Hex, Double, Integer)
   */
  void ScanDigit();

  /**
   * Scan identifier.
   */
  void ScanIdentifier();

  
  void ScanInteger();

  
  void ScanHex();

  
  void ScanOperator();

  
  void ScanArithmeticOperator(Token type1, Token type2, Token normal, bool has_let = true);


  bool ScanUnicodeEscapeSequence(UC32Vector<UChar>*);


  UC16 ScanHexEscape(const UChar& uchar, int len, bool* success);
  

  template <typename T>
  INLINE int ToHexValue(const T& uchar) const {
    int ret = 0;
    if (uchar >= unicode::u8('0') && uchar <= unicode::u8('9')) {
      ret = static_cast<int>(uchar - unicode::u8('0'));
    } else if (uchar >= unicode::u8('a') && uchar <= unicode::u8('f')) {
      ret = static_cast<int>(uchar - unicode::u8('a') + 10);
    } else if (uchar >= unicode::u8('A') && uchar <= unicode::u8('F')) {
      ret = static_cast<int>(uchar - unicode::u8('A') + 10);
    } else {
      return -1;
    }
    return ret;
  }
  

  INLINE bool IsEnd() const {
    return it_ == end_;
  }

  
  INLINE void SkipWhiteSpace() {
    has_line_terminator_before_next_ = false;
    while(IsWhiteSpace(char_) || char_ == unicode::u8(';')) {
      if (char_ == unicode::u8('\n') || char_ == unicode::u8(';')) {
        has_line_terminator_before_next_ = true;
      }
      Advance();
    }
  }
  

  INLINE void ScanLogicalOperator(Token type1, Token type2, Token type3) {
    if (lookahead1_ == char_) {
      return BuildToken(type1);
    }
    if (lookahead1_ == unicode::u8('=')) {
      return BuildToken(type2);
    }
    BuildToken(type3);
  }
  

  void ScanBitwiseOrComparationOperator(
      Token type1, Token type2, Token type3, Token normal, Token equal_comparator, bool has_u = false);

  
  void ScanEqualityComparator(bool not = false);
  

  INLINE void UpdateTokenInfo() {
    token_info_.set_start_col(current_position());
    token_info_.set_line_number(line_number());
  }
  

  INLINE void Error(const char* message) {
    UpdateTokenInfo();
    token_info_.set_type(Token::ILLEGAL);
    std::stringstream str;
    message_ += message;
  }


  INLINE void Illegal() {
    return Error("Illegal token.");
  }


  INLINE void BuildToken(Token type, UC32Vector<UChar> uv) {
    UpdateTokenInfo();
    token_info_.set_value(std::move(uv));
    token_info_.set_type(type);
  }


  INLINE void BuildToken(Token type) {
    UpdateTokenInfo();
    token_info_.set_type(type);
  }
  

  void Advance();


  INLINE size_t current_position() const {
    return current_position_;
  }

  
  INLINE size_t line_number() const {
    return line_number_;
  }
  

  INLINE bool IsIdentifierStart() const {
    return char_.IsAscii() && IsIdentifierStartChar(char_);
  }


  template <typename T>
  INLINE bool IsIdentifierStartChar(T uchar) const {
    return (uchar >= unicode::u8('a') && uchar <= unicode::u8('z')) ||
        (uchar >= unicode::u8('A') && uchar <= unicode::u8('Z')) ||
        (uchar == unicode::u8('_') || uchar == unicode::u8('$'));
  }


  INLINE bool IsUnicodeEscapeSequenceStart() const {
    return char_ == unicode::u8('\\') && lookahead1_ == unicode::u8('u');
  }


  INLINE bool IsInIdentifierRange() const {
    return IsIdentifierStart() || IsNumericLiteral(char_);
  }

  
  INLINE bool IsNumericLiteral(const UChar& uchar) const {
    return uchar.IsAscii() && uchar >= unicode::u8('0') && uchar <= unicode::u8('9');
  }

  
  INLINE bool IsHexRange() const {
    return char_.IsAscii() &&
        (IsNumericLiteral(char_) ||
         (char_ >= unicode::u8('a') && char_ <= unicode::u8('f')) ||
         (char_ >= unicode::u8('A') && char_ <= unicode::u8('F')));
  }

  
  INLINE bool IsStringLiteralStart() const {
    return char_.IsAscii() && (char_ == unicode::u8('\'') || char_ == unicode::u8('"'));
  }

  
  INLINE bool IsDigitStart() const {
    return char_.IsAscii() &&
        ((char_ == unicode::u8('.') && lookahead1_.IsAscii() && IsNumericLiteral(lookahead1_)) ||
         (char_ == unicode::u8('0') && lookahead1_.IsAscii() && lookahead1_ == unicode::u8('x')) ||
         IsNumericLiteral(char_));
  }

  
  INLINE bool IsOperatorStart() const {
    return char_.IsAscii() &&
        (char_ == unicode::u8('+') || char_ == unicode::u8('-') || char_ == unicode::u8('*') ||
         char_ == unicode::u8('/') || char_ == unicode::u8('<') || char_ == unicode::u8('>') ||
         char_ == unicode::u8('|') || char_ == unicode::u8('&') || char_ == unicode::u8('~') ||
         char_ == unicode::u8('^') || char_ == unicode::u8('%') || char_ == unicode::u8('=') ||
         char_ == unicode::u8('!'));
  }

  
  INLINE bool IsWhiteSpace(const UChar& uchar) const {
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


  bool has_line_terminator_before_next_;
  size_t lookahead_cursor_;
  size_t current_position_;
  size_t line_number_;
  InputSourceIterator it_;
  InputSourceIterator end_;
  TokenInfo token_info_;
  UChar char_;
  UChar lookahead1_;
  std::string message_;
};
}


#include "scanner-inl.h"
#endif
