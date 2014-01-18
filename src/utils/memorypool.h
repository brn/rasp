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
#include <boost/thread.hpp>
#include <new>
#include "utils.h"

#ifdef UNIT_TEST
#include <vector>
#endif

#define RASP_ALIGN(offset, alignment) \
  (offset + (alignment - 1)) & ~(alignment - 1)

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



/**
 * The pointer lifetime managable allocator.
 * This memory space is allocation only,
 * it can not free an each space, to free memory block,
 * destroy MemoryPool class.
 */
class MemoryPool : private Uncopyable {
  friend class Allocatable;
 private:
  typedef uint8_t Byte;
  typedef uint16_t TagBit;
  
  static const size_t kAlignment = sizeof(void*);
  static const size_t kPointerSize = RASP_ALIGN(kAlignment, kAlignment);
  static const size_t kAllocatableInterfaceSize = RASP_ALIGN(sizeof(Allocatable), kAlignment);
  static const size_t kTagBitSize = RASP_ALIGN(sizeof(TagBit), kAlignment);
  static const uint16_t kAllocatableTagBit = 0x4000;
  static const uint16_t kFlagMask = 0x3FFF;
  static const uint16_t kMsbMask = 0x7FFF;
  static const uint16_t kSentinelBit = 0x8000;
  static const uint16_t kMaxAllocatableSize = 0x3FFF;


  class Chunk {
   public:
    typedef uint8_t VerificationTag;
    static const size_t kVerificationTagSize = RASP_ALIGN(sizeof(VerificationTag), kAlignment);
    static const uint8_t kVerificationBit = 0xAA;

    /**
     * Instantiate Chunk from heap.
     * @param size The block size.
     */
    RASP_INLINE static Chunk* New(size_t size);


    /**
     * Delete Chunk.
     * @param chunk delete target.
     */
    RASP_INLINE static void Delete(Chunk* chunk);
    
  
    Chunk(Byte* block, size_t size)
        : block_(block),
          block_size_(size),
          used_(0u),
          last_reserved_tag_(nullptr) {}

  
    ~Chunk() = default;


    void Destruct();
  

    /**
     * Check the chunk has the enough size to allocate given size.
     * @param needs needed size.
     * @returns whether the chunk has the enough memory block or not.
     */
    RASP_INLINE bool HasEnoughSize(size_t needs) RASP_NO_SE {
      return block_size_ >= used_ + needs + kTagBitSize;
    }
  

    /**
     * Get memory block.
     * Must call HasEnoughSize before call this,
     * If the given size is over the block capacity,
     * this method cause the segfault.
     * @param needed size.
     * @returns aligned memory chunk.
     */
    inline void* GetBlock(size_t reserve, bool is_allocatable);
  
  
   private :
    RASP_INLINE void set_sentinel_bit() RASP_NOEXCEPT;

  
    RASP_INLINE void unset_sentinel_bit() RASP_NOEXCEPT;

    size_t block_size_;
    Byte* block_;
    size_t used_;
    TagBit* last_reserved_tag_;
  };
  
  
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
  MemoryPool(size_t size);
  MemoryPool(){}

  
  ~MemoryPool() {Destroy();}


  inline void Destroy() {
    if (!deleted_) {
      deleted_ = true;
      Delete(non_class_ptr_head_, [](void* ptr) {free(ptr);});
      Delete(allocatable_head_, [](Allocatable* ptr) {ptr->~Allocatable();free(ptr);});
      Delete(chunk_head_, [](Chunk* ptr) {Chunk::Delete(ptr);});
    }
  }
  
  
  /**
   * allocate to tls space.
   */
  inline static MemoryPool* local_instance(size_t size);

  
  template <typename T>
  T* Alloc(size_t size = 1);


  template <typename T>
  inline T* Allocate(size_t size);
  

#ifdef UNIT_TEST
  std::vector<intptr_t> deleted_allocatable_list;
  std::vector<intptr_t> deleted_non_class_ptr_list;
  std::vector<intptr_t> deleted_chunk_list;
  void ReserveForTest(void* item) {deleted_non_class_ptr_list.push_back(reinterpret_cast<intptr_t>(item));}
  void ReserveForTest(Allocatable* item) {deleted_allocatable_list.push_back(reinterpret_cast<intptr_t>(item));}
  void ReserveForTest(Chunk* item) {deleted_chunk_list.push_back(reinterpret_cast<intptr_t>(item));}
#endif
  
 private :

  /**
   * Allocate the memory and add used block list,
   * if the MemoryPool class is destroyed,
   * all allocated memory is destoryed too.
   * Allocatable size is kDefaultSize.
   */
  inline void* AllocAllocatable(size_t size);


  template <typename T>
  inline void* Alloc(size_t size, bool is_allocatable);


  template <typename T>
  inline void Append(T* pointer);
  

  // The malloced class list.
  SinglyLinkedList<Allocatable>* current_allocatable_;
  SinglyLinkedList<Allocatable>* allocatable_head_;


  SinglyLinkedList<void>* non_class_ptr_head_;
  SinglyLinkedList<void>* current_non_class_ptr_;
  

  // The chunk list.
  SinglyLinkedList<Chunk>* chunk_head_;
  SinglyLinkedList<Chunk>* current_chunk_;
  

  size_t size_;
  bool deleted_;
};

} // namesapce rasp

#include "memorypool-inl.h"
#endif
