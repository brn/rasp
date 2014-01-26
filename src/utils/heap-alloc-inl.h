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

#ifndef UTILS_HEAP_ALLOC_INL_H_
#define UTILS_HEAP_ALLOC_INL_H_

#include "../config.h"

#ifdef HAVE_HEAPALLOC
#include <windows.h>
#include "./utils.h"


namespace rasp {
class InternalHeapAllocator {
 public:
  InternalHeapAllocator() {
    heap_ = HeapCreate(NULL, 0, 0);
    if (heap_ == NULL) {
      throw std::bad_alloc();
    }
  }


  ~InternalHeapAllocator() {
    HeapDestroy(heap_);
  }
  
  
  RASP_INLINE void* Allocate(size_t size) {    
    void* ret = HeapAlloc(heap_, HEAP_ZERO_MEMORY, size);
    if (ret == NULL) {
      throw std::bad_alloc();
    }
    return ret;
  }


  RASP_INLINE void Dealloc(void* area) {
    HeapFree(heap_, NULL, area);
  }
 private:
  HANDLE heap_;
};


RASP_INLINE void* HeapAllocator::Allocate(size_t size) {
  return allocator_->Allocate(size);
}


RASP_INLINE void HeapAllocator::Deallocate(void* ptr) {
  return allocator_->Dealloc(ptr);
}

#endif


inline HeapAllocator::HeapAllocator()
    : allocator_(new InternalHeapAllocator()){}

inline HeapAllocator::~HeapAllocator() {
  delete allocator_;
}


} //namespace rasp
#endif
