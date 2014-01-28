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
#include <atomic>
#include <new>
#include <deque>
#include <algorithm>
#include <mutex>
#include "utils.h"
#include "../config.h"
#include "mmap.h"

#ifdef UNIT_TEST
#include <vector>
#endif

namespace rasp {

class MemoryPool;

class Poolable {
 public:
  inline void* operator new (size_t size, MemoryPool* pool);
  inline void* operator new[] (size_t size, MemoryPool* pool);
  inline void operator delete (void* ptr){};
  inline void operator delete[] (void* ptr){};
  inline void operator delete (void* ptr, MemoryPool* pool);
  inline void operator delete[] (void* ptr, MemoryPool* pool);
  virtual ~Poolable(){}
};


/**
 * The pointer lifetime managable allocator.
 * This memory space is allocation only,
 * it can not free an each space, to free memory block,
 * destroy MemoryPool class.
 */
class MemoryPool : private Uncopyable {
  friend class Poolable;
 private:
  class Chunk;
  struct MemoryBlock;
  
 public :
  explicit MemoryPool(size_t size = 512);

  
  ~MemoryPool() {Destroy();}


  MemoryPool(MemoryPool&& memory_pool) {
    std::swap(*this, memory_pool);
  }
  

  MemoryPool& operator = (MemoryPool&& memory_pool);


  /**
   * Free all allocated memory.
   */
  void Destroy() RASP_NOEXCEPT;
  
  
  /**
   * allocate to tls space.
   */
  inline static MemoryPool* local_instance(size_t size);
  

  /**
   * Free the specified pointer.
   * This function in fact not release memory.
   * This method add memory block to the free list and call destructor.
   * @param object The object pointer that is allocated by MemoryPool::Allocate[Array].
   */
  inline void Dealloc(void* object);
  
 private :


  /**
   * Create an instance from the reserved memory block.
   * @return Specified class instance.
   */
  RASP_INLINE void* Allocate(size_t size);
  

  /**
   * Remove MemoryBlock from the free list.
   * @param find The pointer which is realloced.
   * @param last The pointer which is the prev pointer of the find.
   */
  inline void EraseFromDeallocedList(MemoryPool::MemoryBlock* find, MemoryPool::MemoryBlock* last);

  
  template <typename T>
  RASP_INLINE static void* PtrAdd(T* ptr, size_t size) {
    return reinterpret_cast<void*>(reinterpret_cast<Byte*>(ptr) + size);
  }


  inline void* DistributeBlockWhileLocked(size_t size);


  inline void* DistributeBlockFromChunk(size_t size);
  

  inline void* DistributeBlockFromDeallocedList(size_t size);
  

  inline MemoryPool::MemoryBlock* FindApproximateDeallocedBlock(size_t size);

  
  class Chunk {
   public:
    typedef uint8_t VerificationTag;
    static const size_t kVerificationTagSize = RASP_ALIGN_OFFSET(sizeof(VerificationTag), kAlignment);
    static const uint8_t kVerificationBit = 0xAA;

    /**
     * Instantiate Chunk from heap.
     * @param size The block size.
     */
    inline static Chunk* New(size_t size, Mmap* allocator);


    /**
     * Delete Chunk.
     * @param chunk delete target.
     */
    inline static void Delete(Chunk* chunk) RASP_NOEXCEPT;
    
  
    Chunk(Byte* block, size_t size)
        : block_size_(size),
          used_(0u),
          block_(block),
          tail_block_(nullptr),
          next_(nullptr) {}

  
    ~Chunk() = default;


    void Destruct();
  

    /**
     * Check the chunk has the enough size to allocate given size.
     * @param needs needed size.
     * @returns whether the chunk has the enough memory block or not.
     */
    RASP_INLINE bool HasEnoughSize(size_t needs) RASP_NO_SE;
  

    /**
     * Get memory block.
     * Must call HasEnoughSize before call this,
     * If the given size is over the block capacity,
     * this method cause the segfault.
     * @param needed size.
     * @returns aligned memory chunk.
     */
    inline MemoryBlock* GetBlock(size_t reserve, MemoryPool* pool) RASP_NOEXCEPT;


    RASP_INLINE void set_tail(Byte* block_begin) RASP_NOEXCEPT {
      tail_block_ = block_begin;
    }


    RASP_INLINE bool IsTail(Byte* block_begin) RASP_NOEXCEPT {
      return tail_block_ == block_begin;
    }


    RASP_INLINE void set_next(Chunk* chunk) RASP_NOEXCEPT {next_ = chunk;}


    RASP_INLINE Chunk* next() RASP_NO_SE {return next_;}


    RASP_INLINE size_t size() RASP_NO_SE {return block_size_;}
    
  
   private :

    size_t block_size_;
    size_t used_;
    Byte* block_;
    Byte* tail_block_;
    Chunk* next_;
  };


  class ChunkBundle {
   public:
    ChunkBundle() {}

    
    inline MemoryPool::Chunk* chunk(size_t size, size_t default_size, Mmap* mmap);

    
    inline void Destroy();
    
   private:
    class ChunkList {
     public:
      ChunkList()
          : head_(nullptr),
            current_(nullptr){}

      inline void AllocChunkIfNecessary(size_t size, size_t default_size, Mmap* mmap);

      RASP_INLINE MemoryPool::Chunk* head() RASP_NO_SE {return head_;}
      RASP_INLINE MemoryPool::Chunk* current() RASP_NO_SE {return current_;}
     private:
      MemoryPool::Chunk* head_;
      MemoryPool::Chunk* current_;
    };
    
    RASP_INLINE MemoryPool::Chunk* InitChunk(size_t size, size_t default_size, int index, Mmap* mmap);
    ChunkList bundles_[10];
  };
  

  // The chunk list.
  ChunkBundle* chunk_bundle_;


  MemoryBlock* dealloced_head_;
  MemoryBlock* current_dealloced_;
  

  size_t size_;
  std::atomic<bool> deleted_;
  
  Mmap allocator_;

  std::mutex allocation_mutex_;
  std::mutex deallocation_mutex_;

#ifdef PLATFORM_64BIT
  typedef uint64_t SizeBit;
  typedef uint64_t Size;
  static const uint64_t kTagRemoveBit = 0xFFFFFFFFFFFFFFFC;
  static const uint64_t kMaxAllocatableSize = UINT64_MAX;
  static const uint64_t kDeallocedMask = 0xFFFFFFFFFFFFFFFD;
#elif defined(PLATFORM_32BIT)
  typedef uint32_t SizeBit;
  typedef uint32_t Size;
  static const uint64_t kTagRemoveBit = 0xFFFFFFFC;
  static const uint32_t kMaxAllocatableSize = UINT32_MAX;
  static const uint64_t kDeallocedMask = 0xFFFFFFFD;
#endif

  static const size_t kSizeBitSize = RASP_ALIGN_OFFSET(sizeof(SizeBit), kAlignment);
  static const uint8_t kDeallocedBit = 0x2;
  static const uint32_t kInvalidPointer = 0xDEADC0DE;
  static const size_t kValueOffset = kSizeBitSize + kPointerSize;
};

} // namesapce rasp


#include "memorypool-inl.h"
#endif
