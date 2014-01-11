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


#ifndef ERROR_REPORTER_H_
#define ERROR_REPORTER_H_

#include <iostream>
#include <stdexcept>
#include <sstream>
#include <stdarg.h>
#include "os.h"


#define ERROR_REPORTER_FORMAT__(message, file, line)  \
  std::stringstream ss;                               \
  ss << message << '\n'                               \
  << "in: " << file << '\n'                           \
  << "at: " << line << '\n';


namespace rasp {
template <typename Exception = std::exception>
class ErrorReporter {
 public:
  ErrorReporter(const char* message)
      : message_(message){}
  

  ErrorReporter(const std::string& message):
      message_(std::move(message)){}


  ErrorReporter(const ErrorReporter& e)
      : message_(e.message_){}


  ErrorReporter(const ErrorReporter&& e)
      : message_(std::move(e.message_)){}

  
  void Report(bool is_throw = false) {
    std::cerr << message_ << std::endl;
    if (is_throw) throw Exception(message_.c_str());
  }

 private:
  std::string message_;
};


template <typename T = std::exception>
class MaybeFail {
 public:
  MaybeFail() = default;
  ~MaybeFail() = default;
  
  void AppendError(const char* filename, size_t line_number, const char* error, ...) {
    success_ = false;
    va_list args;
    va_start(args, error);
    std::string f;
    VSPrintf(f, false, error, args);
    ERROR_REPORTER_FORMAT__(f, filename, line_number);
    message_ += ss.str();
  }


  void AppendError(const char* filename, size_t line_number, const std::string& error, ...) {
    success_ = false;
    va_list args;
    va_start(args, error);
    std::string f;
    VSPrintf(f, false, error.c_str(), args);
    ERROR_REPORTER_FORMAT__(f, filename, line_number);
    message_ += ss.str();
  }


  void AppendError(const char* filename, size_t line_number, const T& error) {
    success_ = false;
    ERROR_REPORTER_FORMAT__(error.what(), filename, line_number);
    message_ += ss.str();
  }

  
  bool success() const {
    return success_;
  }

  
  void Throw(const char* error, const char* filename, size_t line_number) {
    success_ = false;
    ERROR_REPORTER_FORMAT__(error, filename, line_number);
    message_ += ss.str();
    ErrorReporter<T> e(message_);
    e.Report(true);
  }


  void Throw(const std::string& error, const char* filename, size_t line_number) {
    success_ = false;
    ERROR_REPORTER_FORMAT__(error, filename, line_number);
    message_ += ss.str();
    ErrorReporter<T> e(message_);
    e.Report(true);
  }


  void Throw() {
    ErrorReporter<T> e(message_);
    e.Report(true);
  }


  ErrorReporter<T>&& GetReporter(const char* error, const char* filename, size_t line_number) {
    success_ = false;
    ERROR_REPORTER_FORMAT__(error, filename, line_number);
    message_ += ss.str();
    return ErrorReporter<T>(message_);
  }


  ErrorReporter<T>&& GetReporter(const std::string& error, const char* filename, size_t line_number) {
    success_ = false;
    ERROR_REPORTER_FORMAT__(error, filename, line_number);
    message_ += ss.str();
    return ErrorReporter<T>(message_);
  }


  ErrorReporter<T>&& GetReporter() {
    return ErrorReporter<T>(message_);
  }
  
 private:
  bool success_;
  std::string message_;
};


#define RASP_APPEND_ERROR(message, ...)                 \
  AppendError(__FILE__, __LINE__, message, __VA_ARGS__)


#define RASP_THROW_ERROR(message, ...)            \
  Throw(__FILE__, __LINE__, message, __VA_ARGS__)


#define RASP_GET_REPORTER(message, ...)                 \
  GetReporter(message, __FILE__, __LINE__, __VA_ARGS__)

} //namespace rasp


#undef ERROR_REPORTER_FORMAT__
#endif
