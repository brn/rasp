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
#include "../config.h"
#include "heap-alloc.h"

#ifdef UNIT_TEST
#include <vector>
#endif

#define RASP_ALIGN(offset, alignment)           \
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
  class Chunk;
  class DisposableBase;
  template <typename T, bool kIsClassType, bool is_array>
  class Disposable;
  struct MemoryBlock;
  
 public :
  explicit MemoryPool(size_t size);

  
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
     Get used memory size of the memory block except the header size.
   * @return The used memory size.
   */
  inline uint64_t CommitedSize() RASP_NO_SE;
  
  
  /**
   * allocate to tls space.
   */
  inline static MemoryPool* local_instance(size_t size);


  /**
   * Create an instance from the reserved memory block.
   * @return Specified class instance.
   */
  template <typename T, typename ... Args>
  RASP_INLINE T* Allocate(Args ... args);


  /**
   * Create an array from the reserved memory block.
   * @return Specified class array.
   */
  template <typename T>
  RASP_INLINE T* AllocateArray(size_t size);


  /**
   * Free the specified pointer.
   * This function in fact not release memory.
   * This method add memory block to the free list and call destructor.
   * @param object The object pointer that is allocated by MemoryPool::Allocate[Array].
   */
  inline void Dealloc(void* object);
  

  // for unit test.
#ifdef UNIT_TEST
  std::vector<intptr_t> deleted_chunk_list;
  void ReserveForTest(Chunk* item) {deleted_chunk_list.push_back(reinterpret_cast<intptr_t>(item));}
#endif
  
 private :

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


  template <typename T, bool is_class_type, bool is_array>
  inline void* Alloc(size_t size, size_t array_size = 0);


  template <typename T, bool is_class_type, bool is_array>
  inline void* AllocFromChunk(size_t size, size_t array_size);
  

  template <typename T, bool is_class_type, bool is_array>
  inline void* ReAllocate(size_t size, size_t array_size);
  

  inline MemoryPool::MemoryBlock* FindApproximateDeallocedBlock(size_t size);

  
  inline void AllocChunkIfNecessary(size_t size);
  

  class Chunk {
   public:
    typedef uint8_t VerificationTag;
    static const size_t kVerificationTagSize = RASP_ALIGN(sizeof(VerificationTag), kAlignment);
    static const uint8_t kVerificationBit = 0xAA;

    /**
     * Instantiate Chunk from heap.
     * @param size The block size.
     */
    static Chunk* New(size_t size, HeapAllocator* allocator);


    /**
     * Delete Chunk.
     * @param chunk delete target.
     */
    inline static void Delete(Chunk* chunk, HeapAllocator* allocator) RASP_NOEXCEPT;
    
  
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
    inline MemoryBlock* GetBlock(size_t reserve) RASP_NOEXCEPT;


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


  class DisposableBase {
   public:
    RASP_INLINE DisposableBase(size_t array_size);
    RASP_INLINE void Dispose(void* block_begin, void* ptr, bool is_free = true) const;
    virtual ~DisposableBase(){}
   protected:
    size_t array_size_;
   private:
    RASP_INLINE virtual void DisposeInternal(void* block_begin, void* ptr, bool is_free) const = 0;
  };
  

  // The chunk list.
  Chunk* chunk_head_;
  Chunk* current_chunk_;


  MemoryBlock* dealloced_head_;
  MemoryBlock* current_dealloced_;
  

  size_t size_;
  bool deleted_;
  
  HeapAllocator allocator_;


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

  static const size_t kPointerSize = kAlignment;
  static const size_t kSizeBitSize = RASP_ALIGN(sizeof(SizeBit), kAlignment);
  static const uint8_t kDeallocedBit = 0x2;
  static const uint32_t kInvalidPointer = 0xDEADC0DE;
  static const size_t kDisposableBaseSize = RASP_ALIGN(sizeof(MemoryPool::DisposableBase), kAlignment);
  static const size_t kDisposableOffset = kSizeBitSize + kPointerSize;
  static const size_t kValueOffset = kDisposableOffset + kDisposableBaseSize;
};

} // namesapce rasp


#include "memorypool-inl.h"
#endif
