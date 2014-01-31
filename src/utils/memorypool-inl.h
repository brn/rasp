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


#ifndef UTILS_POOL_INL_H_
#define UTILS_POOL_INL_H_


#include <stdlib.h>
#include <type_traits>


namespace rasp {


struct MemoryPool::MemoryBlock {

  RASP_INLINE Size size() RASP_NOEXCEPT {
    return *(ToSizeBit());
  }


  RASP_INLINE Size set_size(size_t size) RASP_NOEXCEPT {
    return *(ToSizeBit()) = size;
  }
    
    
  RASP_INLINE SizeBit* ToSizeBit() RASP_NOEXCEPT {
    return reinterpret_cast<SizeBit*>(ToBegin());
  }


  RASP_INLINE MemoryBlock* next_addr() RASP_NOEXCEPT {
    return reinterpret_cast<MemoryBlock*>(ToValue<Byte*>() + size());
  }
    

  RASP_INLINE MemoryBlock* ToNextPtr() RASP_NOEXCEPT {
    return reinterpret_cast<MemoryBlock*>(*(reinterpret_cast<Byte**>(ToBegin() + kSizeBitSize)));
  }


  RASP_INLINE void set_next_ptr(Byte* next_ptr) RASP_NOEXCEPT {
    Byte** next_head = reinterpret_cast<Byte**>(ToBegin() + kSizeBitSize);
    *next_head = next_ptr;
  }
  

  template <typename T = Poolable*>
  RASP_INLINE typename std::remove_pointer<T>::type* ToValue() RASP_NOEXCEPT {
    Pointer p = reinterpret_cast<Pointer>(ToBegin() + kValueOffset);
    return reinterpret_cast<typename std::remove_pointer<T>::type*>(p & kTagRemoveBit);
  }


  RASP_INLINE void MarkAsDealloced() RASP_NOEXCEPT {
    Byte* p = reinterpret_cast<Byte*>(ToBegin() + kValueOffset);
    *p |= kDeallocedBit;
  }


  RASP_INLINE void UnmarkDealloced() RASP_NOEXCEPT {
    Byte* p = reinterpret_cast<Byte*>(ToBegin() + kValueOffset);
    *p &= kDeallocedMask;
  }


  RASP_INLINE bool IsMarkedAsDealloced() RASP_NOEXCEPT {
    return (*reinterpret_cast<Byte*>(ToBegin() + kValueOffset) & kDeallocedBit) == kDeallocedBit;
  }


  RASP_INLINE Byte* ToBegin() RASP_NOEXCEPT {
    return reinterpret_cast<Byte*>(this);
  }
};


inline void* Poolable::operator new(size_t size, MemoryPool* pool) {
  return pool->Allocate(size);
}


inline void* Poolable::operator new[](size_t size, MemoryPool* pool) {
  return pool->Allocate(size);
}


inline void Poolable::operator delete(void* object, MemoryPool* pool) {}


static boost::thread_specific_ptr<MemoryPool> tls_;


// Create Chunk from byte block.
// Chunk and heap block is create from one big memory block.
// The structure is below
// |8-BIT VERIFY BIT|Chunk MEMORY BLOCK|The memory block managed BY Chunk|
inline MemoryPool::Chunk* MemoryPool::Chunk::New(size_t size, Mmap* allocator) {
  ASSERT(true, size <= kMaxAllocatableSize);
  static const size_t kChunkSize = sizeof(Chunk);
  const size_t aligned_size = RASP_ALIGN_OFFSET(size, kAlignment);

#if defined(DEBUG)
  // All heap size we want.
  const size_t heap_size = RASP_ALIGN_OFFSET((kVerificationTagSize + kChunkSize + aligned_size), kAlignment);
#else
  // All heap size we want.
  const size_t heap_size = RASP_ALIGN_OFFSET((kChunkSize + aligned_size), kAlignment);
#endif
  Byte* ptr = reinterpret_cast<Byte*>(allocator->Commit(heap_size));
  
  if (ptr == NULL) {
    throw std::bad_alloc();
  }

#if defined(DEBUG)
  // Verification bit.
  VerificationTag* tag = reinterpret_cast<VerificationTag*>(ptr);
  (*tag) = kVerificationBit;
  void* chunk_area = PtrAdd(ptr, kVerificationTagSize);
#else
  void* chunk_area = ptr;
#endif
  
  // Instantiate Chunk from the memory block.
  return new(chunk_area) Chunk(reinterpret_cast<Byte*>(PtrAdd(chunk_area, kChunkSize)), aligned_size);
}



inline void MemoryPool::Chunk::Delete(Chunk* chunk) RASP_NOEXCEPT {
  chunk->Destruct();
  chunk->~Chunk();
#ifdef DEBUG
  Byte* block = reinterpret_cast<Byte*>(chunk);
  Byte* block_begin = block - kVerificationTagSize;
  ASSERT(true, (*reinterpret_cast<VerificationTag*>(block_begin)) == kVerificationBit);
#endif
}


RASP_INLINE bool MemoryPool::Chunk::HasEnoughSize(size_t needs) RASP_NO_SE {
  return block_size_ >= used_ + (RASP_ALIGN_OFFSET((kValueOffset + needs), kAlignment));
}


// Get heap from the pool.
// The heap structure is bellow
// |1-BIT SENTINEL-FLAG|1-BIT Allocatable FLAG|14-BIT SIZE BIT|FREE-MEMORY|
// This method return FREE-MEMORY area.
inline rasp::MemoryPool::MemoryBlock* MemoryPool::Chunk::GetBlock(size_t reserve) RASP_NOEXCEPT  {
  ASSERT(true, HasEnoughSize(reserve));
  
  Byte* ret = block_ + used_;
  
  if (tail_block_ != nullptr) {
    reinterpret_cast<MemoryBlock*>(tail_block_)->set_next_ptr(ret);
  }
  
  size_t reserved_size = RASP_ALIGN_OFFSET((kValueOffset + reserve), kAlignment);
  size_t real_size = reserve + (reserved_size - (kValueOffset + reserve));
  used_ += reserved_size;
  tail_block_ = ret;
  
  MemoryBlock* memory_block = reinterpret_cast<MemoryBlock*>(ret);
  memory_block->set_size(real_size);
  memory_block->set_next_ptr(nullptr);
  return memory_block;
}


//MemoryPool constructor.
inline MemoryPool::MemoryPool(size_t size)
    : dealloced_head_(nullptr),
      current_dealloced_(nullptr),
      size_(size),
      deleted_(false) {
  ASSERT(true, size <= kMaxAllocatableSize);
  chunk_bundle_ = new(allocator_.Commit(sizeof(ChunkBundle))) ChunkBundle(&allocator_);
}



inline MemoryPool* MemoryPool::local_instance(size_t size) {
  if (tls_.get() == NULL) {
    tls_.reset(new MemoryPool(size));
  }
  return tls_.get();
}


RASP_INLINE void* MemoryPool::Allocate(size_t size) {
  return DistributeBlock(size);
}


inline void MemoryPool::Dealloc(void* object) {  
  Byte* block = reinterpret_cast<Byte*>(object);
  block -= MemoryPool::kValueOffset;
  MemoryPool::MemoryBlock* memory_block = reinterpret_cast<MemoryPool::MemoryBlock*>(block);

  ND_ASSERT(true, !memory_block->IsMarkedAsDealloced());
  
  memory_block->ToValue()->~Poolable();
  memory_block->MarkAsDealloced();
  chunk_bundle_->AddToFreeList(memory_block);
}


inline void* MemoryPool::DistributeBlock(size_t size) {
  const size_t kPoolableSize = sizeof(Poolable);
  MemoryBlock* block = chunk_bundle_->Commit(size, size_);
    
  if (kPoolableSize > size) {
    size = kPoolableSize;
  }
  
  return block->ToValue<void>();
}



inline MemoryPool::MemoryBlock* MemoryPool::ChunkBundle::Commit(size_t size, size_t default_size) {
  ASSERT(true, size > 0);
  int index = FindBestFitBlockIndex(size);
  ChunkList* chunk_list = InitChunk(size, default_size, index);

  if (chunk_list->free_head() != nullptr) {
    if (index != 0) {
      return chunk_list->SwapFreeHead();
    }
    
    MemoryBlock* block = chunk_list->FindApproximateDeallocedBlock(size);
    if (block != nullptr) {
      return block;
    }
  }
  
  return chunk_list->current()->GetBlock(size);
}


RASP_INLINE MemoryPool::MemoryBlock* MemoryPool::ChunkList::SwapFreeHead() RASP_NOEXCEPT {
  MemoryBlock* block = free_head_;
  free_head_ = block->ToNextPtr();
  return block;
}



inline int MemoryPool::ChunkBundle::FindBestFitBlockIndex(size_t size) {
  ND_ASSERT(true, size > 0);
  if (size <= 32) {
    return kSizeMap[size];
  }
  return 0;
}


inline void MemoryPool::ChunkBundle::Destroy() {
  Arena* arena = TlsAlloc();
  for (size_t i = 0u; i < kIndexSizeMap.size() - 1; i++) {
    auto chunk_list = arena->chunk_list(i);
    if (chunk_list->head() != nullptr) {
      auto chunk = chunk_list->head();
      while (chunk != nullptr) {
        ASSERT(true, chunk != nullptr);
        auto tmp = chunk;
        chunk = chunk->next();
        Chunk::Delete(tmp);
      }
    }
  }

  Arena* null_arena = nullptr;
  if (std::atomic_compare_exchange_weak(&arena_head_, &null_arena, arena)) {
    arena_tail_.store(arena, std::memory_order_release);
  } else {
    arena_tail_.load(std::memory_order_acquire)->set_next(arena);
  }
  arena->ReleaseLock();
  tls_.release();
}


inline MemoryPool::ChunkList* MemoryPool::ChunkBundle::InitChunk(size_t size, size_t default_size, int index) {
  ChunkList* chunk_list = TlsAlloc()->chunk_list(index);
  size_t heap_size = index == 0? default_size: RASP_ALIGN_OFFSET(((kIndexSizeMap[index] + (kPointerSize * 2)) * 50), kAlignment);
  chunk_list->AllocChunkIfNecessary(size, heap_size, mmap_);
  return chunk_list;
}


MemoryPool::Arena* MemoryPool::ChunkBundle::TlsAlloc() {
  Arena* arena = nullptr;
  if (arena_head_.load(std::memory_order_relaxed) != nullptr) {
    arena = FindUnlockedArena();
  }

  if (arena == nullptr) {
    arena = tls_.get();
    if (arena == NULL) {
      arena = new(mmap_->Commit(sizeof(Arena))) Arena(mmap_);
      tls_.reset(arena);
    }
  }
  
  return arena;
}


MemoryPool::Arena* MemoryPool::ChunkBundle::FindUnlockedArena() {
  Arena* arena = arena_head_;
  while (arena != nullptr) {
    if (arena->AcquireLock()) {
      return arena;
    }
    arena = arena->next();
  }
  return arena;
}


RASP_INLINE void MemoryPool::ChunkBundle::AddToFreeList(MemoryPool::MemoryBlock* memory_block) RASP_NOEXCEPT {
  ASSERT(true, memory_block->IsMarkedAsDealloced());
  int index = FindBestFitBlockIndex(memory_block->size());
  TlsAlloc()->chunk_list(index)->AppendFreeList(memory_block);
}



inline void MemoryPool::ChunkList::EraseFromDeallocedList(MemoryPool::MemoryBlock* find, MemoryPool::MemoryBlock* last) RASP_NOEXCEPT {
  if (last != nullptr) {
    last->set_next_ptr(find->ToNextPtr()->ToBegin());
  } else {
    free_head_ = find->ToNextPtr();
  }
  find->set_next_ptr(find->next_addr()->ToBegin());
}



inline MemoryPool::MemoryBlock* MemoryPool::ChunkList::FindApproximateDeallocedBlock(size_t size) RASP_NOEXCEPT {
  MemoryBlock* last = nullptr;
  MemoryBlock* find = nullptr;
  MemoryBlock* current = free_head_;
  Size most = MemoryPool::kMaxAllocatableSize;
  
  while (find != nullptr) {
    Size block_size = current->size();
    if (block_size == size) {
      EraseFromDeallocedList(current, last);
      return current;
    }
    if (block_size >= size) {
      Size s = block_size - size;
      if (most > s) {
        most = s;
        find = current;
      }
    }
  }

  if (find != nullptr) {
    EraseFromDeallocedList(find, last);
  }
  return find;
}



inline void MemoryPool::ChunkList::AppendFreeList(MemoryPool::MemoryBlock* block) RASP_NOEXCEPT {
  if (free_head_ == nullptr) {
    free_head_ = current_free_ = block;
  } else {
    current_free_->set_next_ptr(block->ToBegin());
    current_free_ = block;
    block->set_next_ptr(nullptr);
  }
}


inline void MemoryPool::ChunkList::AllocChunkIfNecessary(size_t size, size_t default_size, Mmap* mmap) {
  if (head_ == nullptr) {
    current_ = head_ = MemoryPool::Chunk::New(default_size, mmap);
  }
    
  if (!current_->HasEnoughSize(size)) {
    size_t needs = size > default_size? size + kValueOffset: default_size;
    current_->set_next(MemoryPool::Chunk::New(needs, mmap));
    current_ = current_->next();
  }
}


MemoryPool::Arena::Arena(Mmap* mmap)
    : lock_(ATOMIC_FLAG_INIT),
      mmap_(mmap),
      next_(nullptr) {
  classed_chunk_list_ = reinterpret_cast<ChunkList*>(mmap_->Commit(sizeof(ChunkList) * 4));
  for (int i = 0; i < 4; i++) {
    new(classed_chunk_list_ + i) ChunkList();
  }
}


MemoryPool::Arena::~Arena() {
  for (int i = 0; i < 4; i++) {
    (classed_chunk_list_ + i)->~ChunkList();
  }
}

} // namespace rasp

#endif
