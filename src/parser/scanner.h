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

#include "source.h"
#include "token.h"
#include "../utils/visibility.h"

namespace rasp {

class Scanner {
  public:
    /**
     * @param source The source file content.
     */
    Scanner(Source* source);

    /**
     * Scan the source file from the current position to the next token position.
     */
    Token Scan();


  VISIBLE_FOR_TEST(private) :
    /**
     * Scan string literal.
     */
    Token ScanStringLiteral();

    /**
     * Scan digit literal(includes Hex, Double, Integer)
     */
    Token ScanDigit();

    /**
     * Scan identifier.
     */
    Token ScanIdentifier();


  private:
    Source* source_;
};
}

#endif
