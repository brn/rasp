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

#ifdef UNIT_TEST
#include <vector>
#endif

namespace rasp {
template <size_t kAllocatableSize>
class MemoryPool;


/**
 * @class
 * The base of the lifetime managed pointer.
 * To allocate from the pool,
 * must inherit this class as public.
 */
class Allocatable {
  template <size_t kAllocatableSize>
  friend class MemoryPool;
 public :
  Allocatable(){}
  virtual ~Allocatable(){}
  //The placement new for the pool allocation.
  template <size_t kAllocatableSize>
  void* operator new(size_t size, MemoryPool<kAllocatableSize>* pool);
  void operator delete(void* ptr);
  template <size_t kAllocatableSize>
  void operator delete(void* ptr, MemoryPool<kAllocatableSize>* pool);
};


typedef uint16_t TagBit;
static size_t Align(size_t offset, size_t alignment) {
  return (offset + (alignment - 1)) & ~(alignment - 1);
}
static const size_t kAlignment = sizeof(void*);
static const size_t kAllocatableInterfaceSize = Align(sizeof(Allocatable), kAlignment);
static const size_t kTagBitSize = Align(sizeof(TagBit), kAlignment);
static const uint16_t kAllocatableTagBit = 0x4000;
static const uint16_t kMsbMask = 0x3FFF;
static const uint16_t kExitBit = 0x8000;
static const uint16_t kUnExitBit = 0x7FFF;


template <size_t kSize>
class Chunk {
 public:
  Chunk()
      : used_(0u),
        last_reserved_size_(0u){
    memset(block_, 0, kSize);
  }

  
  ~Chunk() {}


  void Destruct() {
    if (0u == used_) {
      return;
    }
    uint8_t* block = block_;
    size_t used = 0;
    while (1) {
      TagBit* bit_ptr = reinterpret_cast<TagBit*>(block);
      TagBit bit = (*bit_ptr);
      bool exit = (bit & kExitBit) == kExitBit;
      uint16_t size = bit & kMsbMask;
      used += size + kTagBitSize;
      printf("%d %d %d\n", exit, size, used);
      if ((bit & kAllocatableTagBit) == kAllocatableTagBit) {
        reinterpret_cast<Allocatable*>(block + kTagBitSize)->~Allocatable();
      }
      if (exit) {
        break;
      }
      block += Align(size + kTagBitSize, kAlignment);
    }
  };

  
  /**
   * Get next chunk pointer.
   * By default, this method return NULL pointer,
   * if set_next is not called yet.
   * @return next chunk.
   */
  RASP_INLINE Chunk<kSize>* next() RASP_NO_SE { return next_; }

  /**
   * Link next block.
   * Next block only allowed valid pointer.
   * @param chunk empty chunk to link.
   */
  void set_next(Chunk<kSize>* chunk) {
    ASSERT(true, chunk != NULL);
    ASSERT(true, chunk != 0);
    next_ = chunk;
  }

  /**
   * Check the chunk has the enough size to allocate given size.
   * @param needs needed size.
   * @returns whether the chunk has the enough memory block or not.
   */
  RASP_INLINE bool HasEnoughSize(size_t needs) RASP_NO_SE {
    return kSize >= used_ + needs + kTagBitSize;
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
    unset_exit_bit();
    uint8_t* ret = (block_ + used_);
    last_reserved_size_ = (reserve + kTagBitSize);
    if ((kSize - used_) >= last_reserved_size_) {
      used_ += last_reserved_size_;
      used_ = Align(used_, kAlignment);
    }
    set_exit_bit();
    return ret;
  }
  
  
 private :
  void set_exit_bit() {
    TagBit *bit = nullptr;
    if (used_ == 0u) {
      bit = reinterpret_cast<TagBit*>(block_);
    } else {
      bit = reinterpret_cast<TagBit*>(block_ + (used_ - last_reserved_size_));
    }
    (*bit) |= kExitBit;
  }

  
  void unset_exit_bit() {
    TagBit *bit = nullptr;
    if (used_ == 0u) {
      bit = reinterpret_cast<TagBit*>(block_);
    } else {
      bit = reinterpret_cast<TagBit*>(block_ + (used_ - last_reserved_size_));
    }
    (*bit) &= kUnExitBit;
  }
  
  uint8_t block_[kSize];
  size_t used_;
  size_t last_reserved_size_;
};



/**
 * The pointer lifetime managable allocator.
 * This memory space is allocation only,
 * it can not free an each space, to free memory block,
 * destroy MemoryPool class.
 */
template <size_t kAllocatableSize>
class MemoryPool : private Uncopyable {
  friend class Allocatable;
 private:
  static const uint16_t kMaxAllocatableSize = 0x3FFF;
  static_assert(kAllocatableSize <= kMaxAllocatableSize,
                "Exceeded max allocatable size of MemoryPool.");

  template <typename T>
  class SinglyLinkedList {
   public:
    typedef SinglyLinkedList<T> type;
    SinglyLinkedList()
        : next_(nullptr),
          value_(nullptr){}

    
    SinglyLinkedList(T* value)
        : SinglyLinkedList() {
      set_value(value);
    }


    RASP_INLINE void set_value(T* v) {value_ = v;}


    RASP_INLINE T* value() RASP_NO_SE {
      return value_;
    }


    RASP_INLINE void set_next(SinglyLinkedList<T>* next) {
      next_ = next;
    }


    RASP_INLINE SinglyLinkedList<T>* next() RASP_NO_SE {
      return next_;
    }
    
   private:
    T* value_;
    SinglyLinkedList<T>* next_;
  };


  /**
   * free memory space.
   */
  template <typename T, typename Deleter>
  void Delete(T, Deleter);
  
 public :
  MemoryPool();

  
  ~MemoryPool() {Destroy();}


  void Destroy() {
    if (!deleted_) {
      deleted_ = true;
      Delete(non_class_ptr_head_, [](void* ptr) {free(ptr);});
      Delete(allocatable_head_, [](Allocatable* ptr) {ptr->~Allocatable();free(ptr);});
      Delete(chunk_head_, [](Chunk<kAllocatableSize>* ptr) {ptr->Destruct();/*delete ptr;*/});
    }
  }
  
  
  /**
   * allocate to tls space.
   */
  static MemoryPool<kAllocatableSize>* local_instance();

  
  template <typename T>
  T* Alloc(size_t size = 1);


  template <typename T>
  inline T* Allocate(size_t size);


  void SetTag(void*, TagBit* tag, size_t aligned) {
    (*tag) = aligned;
  }
  
  void SetTag(Allocatable*, TagBit* tag, size_t aligned) {
    (*tag) = aligned;
    (*tag) |= kAllocatableTagBit;    
  }
  

#ifdef UNIT_TEST
  std::vector<intptr_t> deleted_allocatable_list;
  std::vector<intptr_t> deleted_non_class_ptr_list;
  std::vector<intptr_t> deleted_chunk_list;
  void ReserveForTest(void* item) {deleted_non_class_ptr_list.push_back(reinterpret_cast<intptr_t>(item));}
  void ReserveForTest(Allocatable* item) {deleted_allocatable_list.push_back(reinterpret_cast<intptr_t>(item));}
  void ReserveForTest(Chunk<kAllocatableSize>* item) {deleted_chunk_list.push_back(reinterpret_cast<intptr_t>(item));}
#endif
  
 private :

  /**
   * Allocate the memory and add used block list,
   * if the MemoryPool class is destroyed,
   * all allocated memory is destoryed too.
   * Allocatable size is kDefaultSize.
   */
  inline void* AllocClassType(size_t size);


  template <bool is_class_type, typename T>
  void* Alloc(size_t size);


  template <typename T>
  void Append(T* pointer);
  

  // The malloced class list.
  SinglyLinkedList<Allocatable>* current_allocatable_;
  SinglyLinkedList<Allocatable>* allocatable_head_;


  SinglyLinkedList<void>* non_class_ptr_head_;
  SinglyLinkedList<void>* current_non_class_ptr_;
  

  // The chunk list.
  SinglyLinkedList<Chunk<kAllocatableSize>>* chunk_head_;
  SinglyLinkedList<Chunk<kAllocatableSize>>* current_chunk_;
  

  bool deleted_;
};

} // namesapce rasp

#include "memorypool-inl.h"
#endif
