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
#include <deque>
#include <algorithm>
#include "utils.h"

#ifdef UNIT_TEST
#include <vector>
#endif

#define RASP_ALIGN(offset, alignment) \
  (offset + (alignment - 1)) & ~(alignment - 1)

namespace rasp {

/**
 * The pointer lifetime managable allocator.
 * This memory space is allocation only,
 * it can not free an each space, to free memory block,
 * destroy MemoryPool class.
 */
class MemoryPool : private Uncopyable {
 private:
  typedef uint8_t Byte;
  typedef uint16_t TagBit;

  class DisposableBase {
   public:
    RASP_INLINE void Dispose(void* block_begin, void* ptr, bool is_free = true) const {
      DisposeInternal(block_begin, ptr, is_free);
    }
    virtual ~DisposableBase(){}
    RASP_INLINE virtual bool IsMalloced() const = 0;
   private:
    RASP_INLINE virtual void DisposeInternal(void* block_begin, void* ptr, bool is_free) const = 0;
  };
  
  static const size_t kAlignment = sizeof(void*);
  static const size_t kPointerSize = RASP_ALIGN(kAlignment, kAlignment);
  static const size_t kTagBitSize = RASP_ALIGN(sizeof(TagBit), kAlignment);
  static const uint16_t kDeallocedBit = 0x4000;
  static const uint16_t kFlagMask = 0x3FFF;
  static const uint16_t kMsbMask = 0x7FFF;
  static const uint16_t kDeallocedBitMask = 0xBFFF;
  static const uint16_t kSentinelBit = 0x8000;
  static const uint16_t kMaxAllocatableSize = 0x3FFF;
  static const size_t kDisposableBaseSize = RASP_ALIGN(sizeof(DisposableBase), kAlignment);


  typedef std::vector<Byte*> DeallocedList;
  

  template <typename T, bool kIsMalloced, bool kIsClassType>
  class Disposable : public MemoryPool::DisposableBase{};


  template <typename T>
  class Disposable<T, false, true> : public MemoryPool::DisposableBase {
   public:
    RASP_INLINE bool IsMalloced() const {return false;}
   private:
    RASP_INLINE virtual void DisposeInternal(void* block_begin, void* ptr, bool is_free) const {
      T* object = reinterpret_cast<T*>(ptr);
      object->~T();
    }
  };


  template <typename T>
  class Disposable<T, true, true> : public MemoryPool::DisposableBase {
   public:
    RASP_INLINE bool IsMalloced() const {return false;}
   private:
    RASP_INLINE virtual void DisposeInternal(void* block_begin, void* ptr, bool is_free) const {
      T* object = reinterpret_cast<T*>(ptr);
      object->~T();
      if (is_free) {
        free(block_begin);
      }
    }
  };


  template <typename T>
  class Disposable<T, false, false> : public MemoryPool::DisposableBase {
   public:
    RASP_INLINE bool IsMalloced() const {return true;}
   private:
    RASP_INLINE virtual void DisposeInternal(void* block_begin, void* ptr, bool is_free) const {}
  };


  template <typename T>
  class Disposable<T, true, false> : public MemoryPool::DisposableBase {
   public:
    RASP_INLINE bool IsMalloced() const {return true;}
   private:
    RASP_INLINE virtual void DisposeInternal(void* block_begin, void* ptr, bool is_free) const {
      if (is_free) {
        free(block_begin);
      }
    }
  };
  

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
    RASP_INLINE static void Delete(Chunk* chunk) RASP_NOEXCEPT;
    
  
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
      size_t offset = kTagBitSize + kDisposableBaseSize;
      return block_size_ >= used_ + needs + offset;
    }
  

    /**
     * Get memory block.
     * Must call HasEnoughSize before call this,
     * If the given size is over the block capacity,
     * this method cause the segfault.
     * @param needed size.
     * @returns aligned memory chunk.
     */
    inline void* GetBlock(size_t reserve) RASP_NOEXCEPT;
  
  
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


    RASP_INLINE void set_value(T* v) RASP_NOEXCEPT {value_ = v;}


    RASP_INLINE T* value() RASP_NO_SE {
      return value_;
    }


    RASP_INLINE void set_next(SinglyLinkedList<T>* next) RASP_NOEXCEPT {
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
  void Delete(T, Deleter) RASP_NOEXCEPT;
  
 public :
  MemoryPool(size_t size);
  MemoryPool(){}

  
  ~MemoryPool() {Destroy();}


  void Destroy() RASP_NOEXCEPT;


  size_t commited_size() const {
    SinglyLinkedList<Chunk>* c = chunk_head_;
    int i = 0;
    while (c != nullptr) {i++;c = c->next();}
    return size_ * i;
  }
  
  
  /**
   * allocate to tls space.
   */
  inline static MemoryPool* local_instance(size_t size);

  
  template <typename T, typename ... Args>
  RASP_INLINE T* Allocate(Args ... args);


  template <typename T>
  RASP_INLINE T* AllocateArray(size_t size);


  void Dealloc(void* object) {
    Byte* block = reinterpret_cast<Byte*>(object);
    block -= (kDisposableBaseSize + kTagBitSize);
    Byte* block_begin = block;
    TagBit* tag = reinterpret_cast<TagBit*>(block);
    DisposableBase* base = reinterpret_cast<DisposableBase*>(block + kTagBitSize);
    base->Dispose(block_begin, block + kTagBitSize + kDisposableBaseSize, false);
    (*tag) |= kDeallocedBit;
    dealloced_list_.push_back(block_begin);
  }
  

#ifdef UNIT_TEST
  std::vector<intptr_t> deleted_malloced_list;
  std::vector<intptr_t> deleted_chunk_list;
  void ReserveForTest(void* item) {deleted_malloced_list.push_back(reinterpret_cast<intptr_t>(item));}
  void ReserveForTest(Chunk* item) {deleted_chunk_list.push_back(reinterpret_cast<intptr_t>(item));}
#endif
  
 private :

  template <typename T>
  RASP_INLINE static void* PtrAdd(T* ptr, size_t size) {
    return reinterpret_cast<void*>(reinterpret_cast<Byte*>(ptr) + size);
  }


  template <typename T, bool is_class_type>
  void* Alloc(size_t size);


  template <typename T, bool is_class_type>
  inline void* AllocFromChunk(size_t size);


  template <typename T, bool is_class_type>
  inline void* AllocFromHeap(size_t size);
  

  inline void Append(void* pointer);


  template <typename T, bool is_class_type>
  inline void* ReAllocate(size_t size);
  

  RASP_INLINE DeallocedList::iterator FindApproximateDeallocedBlock(
      size_t size,
      DeallocedList::iterator& begin,
      DeallocedList::iterator& end);
  
  RASP_INLINE void AllocChunkIfNecessary(size_t size) RASP_NOEXCEPT {
    if (!current_chunk_->value()->HasEnoughSize(size)) {
      current_chunk_->set_next(new SinglyLinkedList<MemoryPool::Chunk>(MemoryPool::Chunk::New(size_)));
      current_chunk_ = current_chunk_->next();
    }
  }
  

  SinglyLinkedList<void>* malloced_head_;
  SinglyLinkedList<void>* current_malloced_;
  

  // The chunk list.
  SinglyLinkedList<Chunk>* chunk_head_;
  SinglyLinkedList<Chunk>* current_chunk_;
  

  size_t size_;
  bool deleted_;
  DeallocedList dealloced_list_;
};

} // namesapce rasp

#include "memorypool-inl.h"
#endif
