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


#ifndef UTILS_UTILS_H_
#define UTILS_UTILS_H_

#include <cstdint>
#include <cstring>
#include "os.h"

namespace rasp {

/**
 * Inline macro.
 */
#if !defined(DEBUG) && defined(_MSC_VER)
#define RASP_INLINE inline __forceinline
#elif !defined(DEBUG) && defined(__GNUC__)
#define RASP_INLINE inline __attribute__((always_inline))
#else
#define RASP_INLINE inline
#endif

#if defined(__GNUC__)
#define RASP_NOEXCEPT noexcept
#else
#define RASP_NOEXCEPT
#endif

#define RASP_NO_SE const RASP_NOEXCEPT


// ASSERT macro definition.
#if defined DEBUG
#if defined __GNUC__

#define ASSERT(expect, result)                                          \
  if ((expect) != (result)) {FPrintf(stderr, "assertion failed -> %s == %s\n in file %s at line %d\n in function %s\n", #result, #expect, __FILE__, __LINE__, __PRETTY_FUNCTION__);abort();}
#define FATAL(msg) FPrintf(stderr, "Fatal error occured, so process no longer exist.\nin file %s at line %d\n in function %s\n%s\n", __FILE__, __LINE__, __PRETTY_FUNCTION__, msg);abort();

#elif defined __func__

#define ASSERT(expect, result) if ((expect) != (result)){FPrintf(stderr, "assertion failed -> %s == %s\n in file %s at line %d\n in function %s\n", #result, #expect, __FILE__, __LINE__, __func__);abort();}
#define FATAL(msg) FPrintf(stderr, "Fatal error occured, so process no longer exist.\nin file %s at line %d\n in function %s\n%s\n", __FILE__, __LINE__, __func__, msg);abort();

#elif defined __FUNC__

#define ASSERT(expect, result) if ((expect) != (result)){FPrintf(stderr, "assertion failed -> %s == %s\n in file %s at line %d\n in function \n", #result, #expect, __FILE__, __LINE__, __FUNC__);abort();}
#define FATAL(msg) FPrintf(stderr, "Fatal error occured, so process no longer exist.\nin file %s at line %d\n in function %s\n%s\n", __FILE__, __LINE__, __FUNC__, msg);abort();

#else
#define ASSERT(expect, result) if ((expect) != (result)){FPrintf(stderr, "assertion failed -> %s == %s\n in file %s at line %d\n", #result, #expect, __FILE__, __LINE__);abort();}
#define FATAL(msg) FPrintf(stderr, "Fatal error occured, so process no longer exist.\nin file %s at line %d\n%s\n", __FILE__, __LINE__, msg);abort();
#endif

#elif defined NDEBUG
#define ASSERT(expect, result)
#define FATAL(msg)
#else
#define ASSERT(expect, result)
#define FATAL(msg)
#endif
// ASSERT macro definition end.


/**
 * Class traits.
 * Represent class which is not allowed to instantiation.
 */
class Static {
  Static() = delete;
  Static(const Static&) = delete;
  Static(Static&&) = delete;
  Static& operator = (const Static&) = delete;
};


/**
 * Class traits.
 * Represent class which is not allowed to copy.
 */
class Uncopyable {
public:
  Uncopyable() = default;
  ~Uncopyable() = default;
  Uncopyable(const Uncopyable&) = delete;
  Uncopyable& operator = (const Uncopyable&) = delete;
};


/**
 * Bitmask utility.
 * Borrowed from http://d.hatena.ne.jp/tt_clown/20090616/p1
 */
template <int LowerBits, typename Type = uint32_t>
class Bitmask {
 public:
  static const Type full = ~(Type(0));
  static const Type upper = ~((Type(1) << LowerBits) - 1);
  static const Type lower = (Type(1) << LowerBits) - 1;
};


/**
 * Generic strlen.
 */
template <typename T>
RASP_INLINE size_t Strlen(const T* str) {
  return strlen(static_cast<const char*>(str));
}

}

#endif
