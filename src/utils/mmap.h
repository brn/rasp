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


#ifndef UTILS_MMAP_H_
#define UTILS_MMAP_H_

#include <atomic>
#include "systeminfo.h"

namespace rasp {
class Mmap {
  class InternalMmap;
 public:
  inline Mmap();


  inline ~Mmap();


  Mmap(Mmap&& mmap) {
    std::swap(*this, mmap);
    mmap.uncommited_.test_and_set();
  }


  Mmap& operator = (Mmap&& mmap) {
    std::swap(*this, mmap);
    mmap.uncommited_.test_and_set();
    return *this;
  }
  
  
  RASP_INLINE void* Commit(size_t size);
  RASP_INLINE void UnCommit();
  RASP_INLINE uint64_t commited_size() RASP_NO_SE;
  RASP_INLINE uint64_t real_commited_size() RASP_NO_SE;

  template <class T>
  class MmapStandardAllocator {
   public:
    typedef size_t  size_type;
    typedef ptrdiff_t difference_type;
    typedef T* pointer;
    typedef const T* const_pointer;
    typedef T& reference;
    typedef const T& const_reference;
    typedef T value_type;

    template <class U>
    struct rebind { 
      typedef MmapStandardAllocator<U> other;
    };

    explicit MmapStandardAllocator(Mmap* mmap)
        : mmap_(mmap){}


    MmapStandardAllocator(const MmapStandardAllocator& allocator)
        : mmap_(allocator.mmap_){}


    template <typename U>
    MmapStandardAllocator(const MmapStandardAllocator<U>& allocator)
        : mmap_(allocator.mmap()){}
    

    /**
     * Allocate new memory.
     */
    pointer allocate(size_type num, const void* hint = 0) {
      return reinterpret_cast<pointer>(mmap_->Commit(sizeof(T) * num));
    }

    /**
     * Initialize already allocated block.
     */
    void construct(pointer p, const T& value) {
      new (static_cast<void*>(p)) T(value);
    }

    /**
     * Return object address.
     */
    pointer address(reference value) const { 
      return &value; 
    }

    /**
     * Return const object address.
     */
    const_pointer address(const_reference value) const { 
      return &value;
    }

    /**
     * Remove pointer.
     */
    void destroy(pointer p) {
      p->~T();
    }

    /**
     * Do nothing.
     */
    void deallocate(pointer p, size_type n) {}

    /**
     * Return the max size of allocatable.
     */
    size_type max_size() const throw() {
      return std::numeric_limits<size_t>::max() / sizeof(T);
    }


    Mmap* mmap() const {return mmap_;}

   private:
    Mmap* mmap_;
  };
  
 private:
  InternalMmap* mmap_;
  std::atomic_flag uncommited_;
};

}

#include "mmap-inl.h"
#endif
