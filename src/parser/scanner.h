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
#include "../utils/visibility.h"

namespace rasp {

class Scanner {
 private:
  typedef Source::Char Char;
 public:
  /**
   * @param source The source file content.
   */
  Scanner(Source* source);

  /**
   * Scan the source file from the current position to the next token position.
   */
  const TokenInfo& Scan();

 private:
  /**
   * Scan string literal.
   */
  const TokenInfo& ScanStringLiteral();

  /**
   * Scan digit literal(includes Hex, Double, Integer)
   */
  const TokenInfo& ScanDigit();

  /**
   * Scan identifier.
   */
  const TokenInfo& ScanIdentifier();

  
  const TokenInfo& ScanInteger();

  
  const TokenInfo& ScanHex();

  
  const TokenInfo& ScanOperator();

  
  const TokenInfo& ScanArithmeticOperator(Token type1, Token type2, Token normal, bool has_let = true);


  INLINE void SkipWhiteSpace() {
    while(IsWhiteSpace()) Advance();
  }
  

  INLINE const TokenInfo& ScanLogicalOperator(Token type1, Token type2, Token type3) {
    if (lookahead1_ == char_) {
      return BuildToken(type1);
    }
    if (lookahead1_ == '=') {
      return BuildToken(type2);
    }
    return BuildToken(type3);
  }
  

  const TokenInfo& ScanBitwiseOrComparationOperator(
      Token type1, Token type2, Token type3, Token normal, Token equal_comparator, bool has_u = false);

  
  const TokenInfo& ScanEqualityComparator(bool not = false);
  

  INLINE void UpdateTokenInfo() {
    token_info_.set_start_col(current_position());
    token_info_.set_line_number(line_number());
    token_info_.set_has_line_break_before_next(lookahead1_ == '\n');
  }
  

  INLINE const TokenInfo& Error(const char* message) {
    UpdateTokenInfo();
    token_info_.set_type(Token::ILLEGAL);
    token_info_.set_value(message);
    return token_info_;
  }


  INLINE const TokenInfo& Illegal() {
    return Error("Illegal token.");
  }


  INLINE const TokenInfo& BuildToken(Token type, const std::stringstream& ss) {
    UpdateTokenInfo();
    const std::string&& val = ss.str();
    token_info_.set_value(val.c_str());
    token_info_.set_type(type);
    return token_info_;
  }


  INLINE const TokenInfo& BuildToken(Token type, const std::string& value) {
    UpdateTokenInfo();
    token_info_.set_value(value.c_str());
    token_info_.set_type(type);
    return token_info_;
  }


  INLINE const TokenInfo& BuildToken(Token type) {
    UpdateTokenInfo();
    token_info_.set_type(type);
    token_info_.set_value("\0");
    return token_info_;
  }
  

  INLINE void Advance() {
    char_ = source_->Advance();
    lookahead1_ = source_->Peek();
  }


  INLINE size_t current_position() const {
    return source_->current_position();
  }

  
  INLINE size_t line_number() const {
    return source_->line_number();
  }
  

  INLINE bool IsIdentifierStart() const {
    return (char_ >= 'a' && char_ <= 'z') ||
        (char_ >= 'A' && char_ <= 'Z') ||
        char_ == '_' || char_ == '$';
  }


  INLINE bool IsInIdentifierRange() const {
    return IsIdentifierStart() || IsNumericLiteral(char_);
  }

  
  INLINE bool IsNumericLiteral(Char ch) const {
    return ch >= '0' && ch <= '9';
  }

  
  INLINE bool IsHexRange() const {
    return IsNumericLiteral(char_) ||
        (char_ >= 'a' && char_ <= 'f') ||
        (char_ >= 'A' && char_ <= 'F');
  }

  
  INLINE bool IsStringLiteralStart() const {
    return char_ == 34 || char_ == 39;
  }

  
  INLINE bool IsDigitStart() const {
    return (char_ == '.' && IsNumericLiteral(lookahead1_)) ||
        (char_ == '0' && lookahead1_ == 'x') ||
        IsNumericLiteral(char_);
  }

  
  INLINE bool IsOperatorStart() const {
    return char_ == '+' || char_ == '-' || char_ == '*' ||
        char_ == '/' || char_ == '<' || char_ == '>' ||
        char_ == '|' || char_ == '&' || char_ == '~' ||
        char_ == '^' || char_ == '%' || char_ == '=' ||
        char_ == '!';
  }

  
  INLINE bool IsWhiteSpace() const {
    return char_ == 0x09 ||
        char_ == 0x0b ||
        char_ == 0x0c ||
        char_ == 0x20 ||
        char_ == 255 ||
        char_ == '\n';
  }

  Source* source_;
  TokenInfo token_info_;
  Char char_;
  Char lookahead1_;
};
}

#endif
