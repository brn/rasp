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
#include <iostream>
#include <unordered_map>
#include <atomic>
#include <new>
#include <deque>
#include <algorithm>
#include "utils.h"
#include "tls.h"
#include "mmap.h"
#include "../config.h"

namespace rasp {

class MemoryPool;


/**
 * Base class of the MemoryPool allocatable object.
 */
class Poolable {
 public:
  /**
   * Create object from MemoryPool allocated memory.
   * The object created by this operator new must not delete or free.
   * If you want to delete object call MemoryPool::Dealloc(void*).
   */
  inline void* operator new (size_t size, MemoryPool* pool);


  /**
   * Create array of object from MemoryPool allocated memory.
   * The object created by this operator new must not delete or free.
   * If you want to delete object call MemoryPool::Dealloc(void*).
   */
  inline void* operator new[] (size_t size, MemoryPool* pool);


  /**
   * DO NOT USE.
   */
  inline void operator delete (void* ptr){UNREACHABLE;};


  /**
   * Used internally.
   */
  inline void operator delete[] (void* ptr){};


  /**
   * Called auto by system if operator new(void*, MemoryPool*) failed.
   */
  inline void operator delete (void* ptr, MemoryPool* pool){};

  
  /**
   * Called auto by system if operator new[](void*, MemoryPool*) failed.
   */
  inline void operator delete[] (void* ptr, MemoryPool* pool){};
  virtual ~Poolable() {}
};


/**
 * Fast thread safe memory allocator.
 * All memory allocateded from heap(mmap/VirtualAlloc).
 * This memory pool allocate object and array which public extends rasp::Poolable,
 * and all allocated object is dealloced by destructor, but if you want,
 * call MemoryPool::Dealloc(void*) explicitly.
 *
 * @example
 * rasp::MemoryPool p(1024);
 * PoolableExtendClass* poolable = new(&p) PoolableExtendClass();
 * // something special.
 * p.Destroy();
 * // If do not call MemoryPool::Destroy(void),
 * // that called in MemoryPool destructor.
 */
class MemoryPool : private Uncopyable {
  // Only Poolable derived class can call
  // MemoryPool::Allocate(size_t) or MemoryPool::AllocateArray(size_t).
  friend class Poolable;
 private:
  class Chunk;
  struct MemoryBlock;
  
 public :
  /**
   * Constructor
   * @param size The hint of each chunk size.
   */
  explicit MemoryPool(size_t size = 512);

  
  ~MemoryPool() {Destroy();}


  MemoryPool(MemoryPool&& memory_pool) {
    std::swap(*this, memory_pool);
  }
  

  MemoryPool& operator = (MemoryPool&& memory_pool);


  /**
   * Free all allocated memory.
   */
  RASP_INLINE void Destroy() RASP_NOEXCEPT;
  

  /**
   * Free the specified pointer.
   * This function in fact not release memory.
   * This method add memory block to the free list and call destructor.
   * @param object The object pointer that must be allocated by MemoryPool::Allocate[Array].
   */
  RASP_INLINE void Dealloc(void* object) RASP_NOEXCEPT;


  /**
   * Get current allocated size by byte.
   * @return The byte expression.
   */
  RASP_INLINE double commited_bytes() RASP_NO_SE {
    return static_cast<double>(allocator_.commited_size());
  }


  /**
   * Get current allocated size by kilo byte.
   * @return The kilo byte expression.
   */
  RASP_INLINE double commited_kbytes() RASP_NO_SE {
    return static_cast<double>(allocator_.commited_size()) / 1024;
  }


  /**
   * Get current allocated size by mega byte.
   * @return The mega byte expression.
   */
  RASP_INLINE double commited_mbytes() RASP_NO_SE {
    return static_cast<double>(allocator_.commited_size()) / 1024 / 1024;
  }


  /**
   * Get current allocated heap size which include all unused space.
   * @return The byte expression.
   */
  RASP_INLINE double real_commited_bytes() RASP_NO_SE {
    return static_cast<double>(allocator_.real_commited_size());
  }


  /**
   * Get current allocated heap size which include all unused space.
   * @return The kilo byte expression.
   */
  RASP_INLINE double real_commited_kbytes() RASP_NO_SE {
    return static_cast<double>(allocator_.real_commited_size()) / 1024;
  }


  /**
   * Get current allocated heap size which include all unused space.
   * @return The mega byte expression.
   */
  RASP_INLINE double real_commited_mbytes() RASP_NO_SE {
    return static_cast<double>(allocator_.real_commited_size()) / 1024 / 1024;
  }
  
 private :


  /**
   * Return enough size memory block for specified size.
   * @return Unused memory block.
   */
  RASP_INLINE void* Allocate(size_t size);


  /**
   * Return enough size memory block for specified size.
   * @return Unused memory block.
   */
  RASP_INLINE void* AllocateArray(size_t size);


  /**
   * Advance pointer position
   */
  template <typename T>
  RASP_INLINE static void* PtrAdd(T* ptr, size_t size) {
    return reinterpret_cast<void*>(reinterpret_cast<Byte*>(ptr) + size);
  }


  /**
   * Destruct Poolable class instance by the proper method.
   */
  RASP_INLINE static void DestructMemoryBlock(MemoryPool::MemoryBlock* memory_block);


  /**
   * Allocate unused memory space from chunk.
   */
  inline MemoryPool::MemoryBlock* DistributeBlock(size_t size);


  /**
   * The memory block representation class.
   */
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
    

    /**
     * Constructor
     * @param block Unused block allocated by Mmap.
     * @param size block size.
     */
    Chunk(Byte* block, size_t size)
        : block_size_(size),
          used_(0u),
          block_(block),
          tail_block_(nullptr),
          next_(nullptr) {}

  
    ~Chunk() = default;


    /**
     * Remove all chunks.
     */
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
    MemoryBlock* GetBlock(size_t reserve) RASP_NOEXCEPT;


    /**
     * Store last memory block.
     * @param block_begin The last allocated block.
     */
    RASP_INLINE void set_tail(Byte* block_begin) RASP_NOEXCEPT {
      tail_block_ = block_begin;
    }


    /**
     * Check wheter the given block is tail of the chunk or not.
     * @param block_begin New memory block.
     */
    RASP_INLINE bool IsTail(Byte* block_begin) RASP_NOEXCEPT {
      return tail_block_ == block_begin;
    }


    /**
     * Connect new chunk to the next link.
     * @param chunk New chunk.
     */
    RASP_INLINE void set_next(Chunk* chunk) RASP_NOEXCEPT {next_ = chunk;}


    /**
     * Return next link.
     */
    RASP_INLINE Chunk* next() RASP_NO_SE {return next_;}


    /**
     * Return current chunk size.
     */
    RASP_INLINE size_t size() RASP_NO_SE {return block_size_;}
    
  
   private :

    size_t block_size_;
    size_t used_;
    Byte* block_;
    Byte* tail_block_;
    Chunk* next_;
  };


  /**
   * The linked list of chunk.
   */
  class ChunkList {
   public:
    ChunkList()
        : head_(nullptr),
          current_(nullptr),
          free_head_(nullptr),
          current_free_(nullptr) {}


    /**
     * Return head of list.
     */
    RASP_INLINE MemoryPool::Chunk* head() RASP_NO_SE {return head_;}


    /**
     * Return tail of list.
     */
    RASP_INLINE MemoryPool::Chunk* current() RASP_NO_SE {return current_;}


    /**
     * Return head of free list.
     */
    RASP_INLINE MemoryPool::MemoryBlock* free_head() RASP_NO_SE {return free_head_;}


    /**
     * Return tail of free list.
     */
    RASP_INLINE MemoryPool::MemoryBlock* current_free() RASP_NO_SE {return current_free_;}


    /**
     * Connect memory block to tail of free list and replace tail.
     * @param block Dealloced memory block.
     */
    inline void AppendFreeList(MemoryPool::MemoryBlock* block) RASP_NOEXCEPT;
      

    /**
     * Find the most nearly size block from free list if size class is 0.
     * @param size Need
     */
    MemoryPool::MemoryBlock* FindApproximateDeallocedBlock(size_t size) RASP_NOEXCEPT;


    /**
     * Create new chunk and connect
     * if current chunk not has enough size to allocate given size.
     * @param size Need size
     * @param default_size default size of the chunk if size class is zero
     * @param mmap allocator
     */
    inline void AllocChunkIfNecessary(size_t size, size_t default_size, Mmap* mmap);


    /**
     * Remove MemoryBlock from the free list.
     * @param find The pointer which is realloced.
     * @param last The pointer which is the prev pointer of the find.
     */
    inline void EraseFromDeallocedList(MemoryPool::MemoryBlock* find, MemoryPool::MemoryBlock* last) RASP_NOEXCEPT;


    /**
     * Swap head of free list to next and return last head.
     */
    RASP_INLINE MemoryPool::MemoryBlock* SwapFreeHead() RASP_NOEXCEPT;
      
   private:
    MemoryPool::Chunk* head_;
    MemoryPool::Chunk* current_;
    MemoryPool::MemoryBlock* free_head_;
    MemoryPool::MemoryBlock* current_free_;
  };


  class LocalArena;
  

  /**
   * The arena which is allocated in global space.
   */
  class CentralArena {
   public:
    /**
     * Constructor
     * @param mmap allocator
     */
    CentralArena(Mmap* mmap)
        : arena_head_(nullptr),
          arena_tail_(nullptr),
          mmap_(mmap) {
      tls_ = new(mmap_->Commit(sizeof(ThreadLocalStorage::Slot))) ThreadLocalStorage::Slot(&TlsFree);
    }


    /**
     * Get an arena from tls or allocate new one.
     * @param size Need size.
     * @param default_size Default chunk size.
     */
    inline MemoryPool::MemoryBlock* Commit(size_t size, size_t default_size);


    /**
     * Remove all arena.
     */
    void Destroy();


    /**
     * Deallocate specified ptr.
     * @param object The object which want to deallocate.
     */
    void Dealloc(void* object);


    /**
     * Unlock arena.
     * @param arena The arena which want to unlock.
     */
    RASP_INLINE void FreeArena(MemoryPool::LocalArena* arena);
    
   private:


    /**
     * Return index of chunk list which fit to given size.
     * @param size Need size.
     */
    inline int FindBestFitBlockIndex(size_t size);


    /**
     * Find out arena which was unlocked.
     */
    inline LocalArena* FindUnlockedArena();
    

    /**
     * Initialize chunk.
     * @param size Need size.
     * @param default_size Default chunk size.
     * @param index The class of arena.
     */
    RASP_INLINE MemoryPool::ChunkList* InitChunk(size_t size, size_t default_size, int index);


    /**
     * Allocate arena to tls or get unlocked arena.
     * @return Current thread local arena.
     */
    RASP_INLINE MemoryPool::LocalArena* TlsAlloc();
    

    /**
     * Add an arena to linked list.
     * @param arena The arena which want to connect.
     */
    inline void StoreNewLocalArena(MemoryPool::LocalArena* arena);
    

    LocalArena* arena_head_;
    LocalArena* arena_tail_;

    Mmap* mmap_;
    SpinLock lock_;
    SpinLock dealloc_lock_;
    SpinLock tree_lock_;
    ThreadLocalStorage::Slot* tls_;
    
    static const int kSmallMax = 3 KB;

    static inline void TlsFree(void* arena) {
      reinterpret_cast<MemoryPool::LocalArena*>(arena)->Return();
    }
  };


  /**
   * The thread local arena.
   */
  class LocalArena {
   public:
    /**
     * Constructor
     * @param central_arena The central arena.
     * @param mmap Allocator
     */
    inline LocalArena(CentralArena* central_arena, Mmap* mmap);
    inline ~LocalArena();


    /**
     * Return next MemoryPool::LocalArena.
     */
    RASP_INLINE LocalArena* next() RASP_NO_SE {
      return next_;
    }


    /**
     * Append MemoryPool::LocalArena to next link.
     * @param An arena which want to connect.
     */
    RASP_INLINE void set_next(LocalArena* arena) RASP_NOEXCEPT {
      next_ = arena;
    }

    
    /**
     * Try lock arena.
     */
    RASP_INLINE bool AcquireLock() RASP_NOEXCEPT {
      return !lock_.test_and_set();
    }


    /**
     * Unlock arena.
     */
    RASP_INLINE void ReleaseLock() RASP_NOEXCEPT {
      lock_.clear();
    }


    /**
     * Get chunk which belong to given class index.
     * @param index
     * @return Specific class chunk list.
     */
    RASP_INLINE ChunkList* chunk_list(int index) {
      return classed_chunk_list_ + index;
    }


    /**
     * Add The MemoryPool::LocalArena to free list of The MemoryPool::CentralArena.
     */
    RASP_INLINE void Return();
   private:
    CentralArena* central_arena_;
    std::atomic_flag lock_;
    Mmap* mmap_;
    ChunkList* classed_chunk_list_;
    LocalArena* next_;
  };
  

  // The chunk list.
  CentralArena* central_arena_;


  MemoryBlock* dealloced_head_;
  MemoryBlock* current_dealloced_;
  

  size_t size_;
  std::atomic_flag deleted_;
  SpinLock tree_lock_;
  
  Mmap allocator_;

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
  static const uint8_t kArrayBit = 0x1;
  static const uint32_t kInvalidPointer = 0xDEADC0DE;
  static const size_t kValueOffset = kSizeBitSize + kPointerSize;
  static const int kMaxSmallObjectsCount = 30;
};

} // namesapce rasp


#include "memorypool-inl.h"
#endif
