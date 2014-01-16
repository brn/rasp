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

#include "./scanner-test-prelude.h"

#define KEYWORD_TEST(keyword, token_type)                       \
  TEST(ScannerTest, ScanIdentifier_keyword_##keyword)           \
  {                                                             \
    INIT(token, #keyword);                                      \
    ASSERT_STREQ(#token_type, token->ToString());                \
    ASSERT_EQ(rasp::Token::token_type, token->type());           \
    rasp::Utf8Value utf8 = token->value().ToUtf8Value();         \
    ASSERT_STREQ(utf8.value(), #keyword);                       \
    ASSERT_EQ(utf8.size(), strlen(#keyword));                   \
    END_SCAN;                                                   \
  }


#define RESERVED_KW_TEST(keyword)               \
  KEYWORD_TEST(keyword, FUTURE_RESERVED_WORD)


#define STRICT_RESERVED_KW_TEST(keyword)        \
  KEYWORD_TEST(keyword, FUTURE_STRICT_RESERVED_WORD)


KEYWORD_TEST(break, JS_BREAK);
KEYWORD_TEST(case, JS_CASE);
KEYWORD_TEST(catch, JS_CATCH);
RESERVED_KW_TEST(class);
KEYWORD_TEST(const, JS_CONST);
KEYWORD_TEST(continue, JS_CONTINUE);
KEYWORD_TEST(debugger, JS_DEBUGGER);
KEYWORD_TEST(default, JS_DEFAULT);
KEYWORD_TEST(delete, JS_DELETE);
KEYWORD_TEST(do, JS_DO);
KEYWORD_TEST(else, JS_ELSE);
RESERVED_KW_TEST(enum);
RESERVED_KW_TEST(export);
RESERVED_KW_TEST(extends);
KEYWORD_TEST(false, JS_FALSE);
KEYWORD_TEST(finally, JS_FINALLY);
KEYWORD_TEST(for, JS_FOR);
KEYWORD_TEST(function, JS_FUNCTION);
KEYWORD_TEST(if, JS_IF);
STRICT_RESERVED_KW_TEST(implements);
RESERVED_KW_TEST(import);
KEYWORD_TEST(in, JS_IN);
KEYWORD_TEST(instanceof, JS_INSTANCEOF);
STRICT_RESERVED_KW_TEST(interface);
STRICT_RESERVED_KW_TEST(let);
KEYWORD_TEST(NaN, JS_NAN);
KEYWORD_TEST(new, JS_NEW);
KEYWORD_TEST(null, JS_NULL);
STRICT_RESERVED_KW_TEST(package);
STRICT_RESERVED_KW_TEST(private);
STRICT_RESERVED_KW_TEST(protected);
STRICT_RESERVED_KW_TEST(public);
KEYWORD_TEST(return, JS_RETURN);
STRICT_RESERVED_KW_TEST(static);
RESERVED_KW_TEST(super);
KEYWORD_TEST(switch, JS_SWITCH);
KEYWORD_TEST(this, JS_THIS);
KEYWORD_TEST(throw, JS_THROW);
KEYWORD_TEST(true, JS_TRUE);
KEYWORD_TEST(try, JS_TRY);
KEYWORD_TEST(typeof, JS_TYPEOF);
KEYWORD_TEST(undefined, JS_UNDEFINED);
KEYWORD_TEST(var, JS_VAR);
KEYWORD_TEST(void, JS_VOID);
KEYWORD_TEST(while, JS_WHILE);
KEYWORD_TEST(with, JS_WITH);
KEYWORD_TEST(yield, JS_YIELD);


