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

#include <parser/source>
#include <parser/tokenlist>
#include <boost/concept/assert.hpp>
#include <concepts/token-list-concept>
#include <concepts/source-concept>


namespace rasp {
template <typename Source, typename TokenList>
class Scanner {
 public :
  //Scanner require TokenReservable concepts.
  BOOST_CONCEPT_ASSERT((rasp::TokenReservable<TokenList>));
  //Scanner require SourceReservable concepts.
  BOOST_CONCEPT_ASSERT((rasp::SourceReservable<Source>));
  
  Scanner(Source source, TokenList tokenList);
  void Tokenize();
 private :
  Source source_;
  TokenList tokenList_;
}
}
