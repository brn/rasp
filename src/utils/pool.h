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


#ifndef UTILS_POOL_H_
#define UTILS_POOL_H_

#include <stdio.h>
#include <stdlib.h>
#include "utils.h"


namespace rasp {
class MemoryPool;


/**
 * @class
 * The base of the lifetime managed pointer.
 * To allocate from the pool,
 * must inherit this class as public.
 */
class Allocatable {
  friend class MemoryPool;
 public :
  Allocatable(){}
  virtual ~Allocatable(){}
  //The placement new for the pool allocation.
  void* operator new(size_t size, MemoryPool* pool);
  void operator delete(void* ptr);
  void operator delete(void* ptr, MemoryPool* pool);
};


template <size_t>
class Chunk {
 public:
  Chunk()
      : used_(0),
        next_(NULL) {
    memset(block_, 0, size);
  }

  
  ~Chunk();

  
  /**
   * Get next chunk pointer.
   * By default, this method return NULL pointer,
   * if set_next is not called yet.
   * @return next chunk.
   */
  Chunk<size>* next() { return next_; }

  /**
   * Link next block.
   * Next block only allowed valid pointer.
   * @param chunk empty chunk to link.
   */
  void set_next(Chunk<size>* chunk) {
    ASSERT(true, chunk != NULL);
    ASSERT(true, chunk != 0);
    next_ = chunk;
  }

  /**
   * Check the chunk has the enough size to allocate given size.
   * @param needs needed size.
   * @returns whether the chunk has the enough memory block or not.
   */
  bool HasEnoughSize(size_t needs) {
    return (size - used_) >= needs;
  }

  /**
   * Get memory block.
   * Must call HasEnoughSize before call this,
   * If the given size is over the block capacity,
   * this method cause the segfault.
   * @param needed size.
   * @returns aligned memory chunk.
   */
  void* GetBlock(size_t reserve) {
    ASSERT(true, HasEnoughSize(reserve));
    char* ret = (block_ + used_);
    used_ += reserve;
    used_ = MemoryPool::Align(used_, 8);
    return ret;
  }

  
 private :
  uint8_t block_[size];
  size_t used_;
  Chunk<size>* next_;
};



/**
 * The pointer lifetime managable allocator.
 * This memory space is allocation only,
 * it can not free an each space, to free memory block,
 * destroy MemoryPool class.
 */
template <size_t AllocatableSize>
class MemoryPool : private Uncopyable {
 private:
  friend class Allocatable;

  template <typename T>
  class SinglyLinkedList {
   public:
    SinglyLinkedList()
        : prev(nullptr),
          next(nullptr) {}


    RASP_INLINE void set_value(T* v) {value_ = v;}


    RASP_INLINE T* value() RASP_NO_SE {
      return value_;
    }


    RASP_INLINE void set_next(SinglyLinkedList<T>* next) {
      next_ = next;
    }


    RASP_INLINE SinglyLinkedList<T*> next() RASP_NO_SE {
      return next_;
    }
    
   private:
    T* value_;
    SinglyLinkedList<T>* next_;
  };


  /**
   * free memory space.
   */
  template <bool is_class_type>
  class Deleter {
   public:
    template <typename T>
    static void Delete(MemoryPool<AllocatableSize>::SinglyLinkedList<T>*,
                       MemoryPool<AllocatableSize>::SinglyLinkedList<T>*);
  };
  
 public :
  MemoryPool();

  
  ~MemoryPool() {
    Deleter<false>::Delete(non_class_ptr_head_,
                          current_non_class_ptr_);
    Deleter<true>::Delete(allocatable_head_,
                          current_allocatable_);
    Deleter<true>::Delete(chunk_head_,
                          current_chunk_);
  }

  
  /**
   * allocate to tls space.
   */
  static MemoryPool* LocalInstance();

  
  static size_t Align(size_t offset, size_t alignment);

  
  template <typename T>
  T* Alloc(size_t size = 1);

  
 private :
  void* AllocPrimitive(size_t size);

  /**
   * Allocate the memory and add used block list,
   * if the MemoryPool class is destroyed,
   * all allocated memory is destoryed too.
   * Allocatable size is kDefaultSize.
   */
  void* AllocClassType(size_t size);


  template <T>
  void Append(T* allocated_list);
  

  // The malloced class list.
  SinglyLinkedList<Allocatable>* current_allocatable_;
  SinglyLinkedList<Allocatable>* allocatable_head_;


  SinglyLinkedList<void>* non_class_ptr_head_;
  SinglyLinkedList<void>* current_non_class_ptr_;
  

  // The chunk list.
  SinglyLinkedList<Chunk<AllocatableSize>>* chunk_head_;
  SinglyLinkedList<Chunk<AllocatableSize>>* current_chunk_;

  static size_t kAllocatableSize = AllocatableSize;
  
  static thread_local *MemoryPool local_instance_;
};

} // namesapce rasp

#include "pool-inl.h"
#endif
