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


#ifndef UTILS_MMAP_INL_H_
#define UTILS_MMAP_INL_H_

#include "./utils.h"
#include "../config.h"


#if defined(HAVE_MMAP)
#include "mmap-mmap.h"
#elif defined(HAVE_VIRTUALALLOC)
#include "mmap-virtual-alloc.h"
#endif


namespace rasp {
  
class Mmap::InternalMmap {
 private:
  
  class Header {
   public:
    RASP_INLINE Header* ToNextPtr() RASP_NO_SE {
      return reinterpret_cast<Header*>(*reinterpret_cast<Byte**>(ToBegin()));
    }


    RASP_INLINE void set_next(Header* byte) RASP_NO_SE {
      Header** next = reinterpret_cast<Header**>(ToBegin());
      *next = byte;
    }


    RASP_INLINE Byte* ToValue() RASP_NO_SE {
      return ToBegin() + kPointerSize;
    }
  

    RASP_INLINE Byte* ToBegin() RASP_NO_SE {
      return reinterpret_cast<Byte*>(const_cast<Header*>(this));
    }
  };

  
 public:
  RASP_INLINE InternalMmap():
      current_map_size_(kDefaultByte),
      used_(0u),
      heap_(nullptr),
      current_(nullptr),
      last_(nullptr) {}


  ~InternalMmap() = default;
  
  
  RASP_INLINE void* Commit(size_t size) {
    size_t needs = RASP_ALIGN_OFFSET((kPointerSize + size), 4 KB);
    if (current_map_size_ < needs || (current_map_size_ - used_) < needs || heap_ == nullptr) {
      return Alloc(needs);
    }
    Byte* ret = reinterpret_cast<Byte*>(current_) + used_;
    used_ += size;
    return static_cast<void*>(ret);
  }


  RASP_INLINE void UnCommit() {
    Header* area = reinterpret_cast<Header*>(heap_);
    while (area != nullptr) {
      Header* tmp = area->ToNextPtr();
      MapAllocator::Deallocate(area->ToBegin());
      area = tmp;
    }
  }

 private:
  
  RASP_INLINE void* Alloc(size_t size) {
    size_t map_size = kDefaultByte;
    if (size > kDefaultByte) {
      map_size = size;
    }
    current_map_size_ = map_size;
    void* heap;
    if (heap_ == nullptr) {
      heap = current_ = heap_ = MapAllocator::Allocate(map_size);
    } else {
      heap = current_ = MapAllocator::Allocate(map_size);
    }
    used_ = 0u;
    return AddHeader(heap, size);
  }


  RASP_INLINE void* AddHeader(void* heap, size_t size) {
    Header* header = reinterpret_cast<Header*>(heap);
    Byte** next_ptr = reinterpret_cast<Byte**>(heap);
    next_ptr = nullptr;
    if (last_ != nullptr) {
      last_->set_next(header);
    }
    used_ += size;
    last_ = header;
    return static_cast<void*>(header->ToValue());
  }
  

  size_t current_map_size_;
  size_t used_;
  void* heap_;
  void* current_;
  Header* last_;
};


RASP_INLINE void* Mmap::Commit(size_t size) {
  return mmap_->Commit(size);
}


RASP_INLINE void Mmap::UnCommit() {
  if (!uncommited_.load()) {
    uncommited_.store(true);
    mmap_->UnCommit();
  }
}


Mmap::Mmap()
    : mmap_(new InternalMmap()),
      uncommited_(false) {}

Mmap::~Mmap() {
  UnCommit();
}

} // namespace rasp

#endif
