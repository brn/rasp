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


#ifndef PARSER_UCHAR_VECTOR_H_
#define PARSER_UCHAR_VECTOR_H_

#include <numeric>
#include <string>
#include <vector>
#include "../utils/inline.h"
#include "../utils/unicode.h"

namespace rasp {

template <typename T>
class UC32Vector {
 public:
  typedef std::vector<T> ValueType;
  
  UC32Vector(size_t size = 0) {
    if (size > 0u) {
      vector_.resize(size);
    }
  };

  
  ~UC32Vector() = default;

  
  UC32Vector(const UC32Vector<T>& vector)
      : vector_(vector.vector_){}

  
  UC32Vector(UC32Vector<T>&& vector)
      : vector_(std::move(vector.vector_)){}


  INLINE UC32Vector& operator = (UC32Vector<T>&& data) {
    vector_ = std::move(data.vector_);
    return (*this);
  }


  INLINE const UC32Vector& operator += (const UC32Vector<T>& data) {
    for (auto v : data.vector_) {
      vector_.push_back(v);
    }
    return (*this);
  }


  INLINE const UC32Vector& operator += (const T& data) {
    vector_.push_back(data);
    return (*this);
  }


  INLINE const UC32Vector operator + (const UC32Vector<T>& data) {
    UC32Vector<T> uv((*this));
    for (auto v : data.vector_) {
      uv.vector_.push_back(v);
    }
    return uv;
  }


  INLINE const UC32Vector operator + (const T& data) {
    UC32Vector<T> uv((*this));
    uv.vector_.push_back(data);
    return uv;
  }
  
  
  INLINE const ValueType& data() const {
    return vector_;
  }


  INLINE Utf8Value ToUtf8Value() const {
    Utf8Value value;
    value.reserve(utf8_length());
    for (const T& t : vector_) {
      value.append(t.utf8());
    }
    return value;
  }


  INLINE Utf16Value ToUtf16Value() const {
    Utf16Value value;
    value.reserve(utf16_length());
    for (const T& t : vector_) {
      if (t.IsSurrogatePair()) {
        value.append(1, t.ToHighSurrogate());
        value.append(1, t.ToLowSurrogate());
      } else {
        value.append(1, t.uchar());
      }
    }
    return value;
  }


  INLINE size_t utf8_length() const {
    return std::accumulate(vector_.begin(), vector_.end(), 0u,
                           [](size_t sum, const UChar& uchar) {
                             return sum + uchar.utf8_length();
                           });
  }


  INLINE size_t utf16_length() const {
    return std::accumulate(vector_.begin(), vector_.end(), 0u,
                           [](size_t sum, const UChar& uchar) {
                             return sum + uchar.utf16_length();
                           });
  }
  
 private:
  size_t utf8_length_;
  size_t utf16_length_;
  ValueType vector_;
  
};

} //namespace rasp

#endif PARSER_UCHAR_VECTOR_H_
