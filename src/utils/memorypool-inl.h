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
#include <thread>


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


  RASP_INLINE void MarkAsArray() RASP_NOEXCEPT {
    Byte* p = reinterpret_cast<Byte*>(ToBegin() + kValueOffset);
    *p |= kArrayBit;
  }


  RASP_INLINE bool IsMarkedAsArray() RASP_NOEXCEPT {
    return (*reinterpret_cast<Byte*>(ToBegin() + kValueOffset) & kArrayBit) == kArrayBit;
  }


  RASP_INLINE Byte* ToBegin() RASP_NOEXCEPT {
    return reinterpret_cast<Byte*>(this);
  }
};


inline void* Poolable::operator new(size_t size, MemoryPool* pool) {
  return pool->Allocate(size);
}


inline void* Poolable::operator new[](size_t size, MemoryPool* pool) {
  return pool->AllocateArray(size);
}


void MemoryPool::Destroy() RASP_NOEXCEPT {
  if (!deleted_.test_and_set()) {
    central_arena_->Destroy();
  }
}



void MemoryPool::Dealloc(void* object) RASP_NOEXCEPT {
  central_arena_->Dealloc(object);
}


void* MemoryPool::Allocate(size_t size) {
  return DistributeBlock(size)->ToValue<void>();
}


void* MemoryPool::AllocateArray(size_t size) {
  MemoryBlock* block = DistributeBlock(size);
  block->MarkAsArray();
  return block->ToValue<void>();
}


void MemoryPool::DestructMemoryBlock(MemoryPool::MemoryBlock* memory_block) {
  if (!memory_block->IsMarkedAsArray()) {
    memory_block->ToValue()->~Poolable();
  } else {
    delete[] memory_block->ToValue();
  }
}


MemoryPool::MemoryBlock* MemoryPool::DistributeBlock(size_t size) {
  const size_t kPoolableSize = sizeof(Poolable);
  MemoryBlock* block = central_arena_->Commit(size, size_);
    
  if (kPoolableSize > size) {
    size = kPoolableSize;
  }
  
  return block;
}


// Create Chunk from byte block.
// Chunk and heap block is create from one big memory block.
// The structure is below
// |8-BIT VERIFY BIT|Chunk MEMORY BLOCK|The memory block managed BY Chunk|
MemoryPool::Chunk* MemoryPool::Chunk::New(size_t size, Mmap* allocator) {
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


void MemoryPool::Chunk::Delete(Chunk* chunk) RASP_NOEXCEPT {
  chunk->Destruct();
  chunk->~Chunk();
#ifdef DEBUG
  Byte* block = reinterpret_cast<Byte*>(chunk);
  Byte* block_begin = block - kVerificationTagSize;
  ASSERT(true, (*reinterpret_cast<VerificationTag*>(block_begin)) == kVerificationBit);
#endif
}


bool MemoryPool::Chunk::HasEnoughSize(size_t needs) RASP_NO_SE {
  return block_size_ >= used_ + (RASP_ALIGN_OFFSET((kValueOffset + needs), kAlignment));
}
// Chunk inline end


// ChunkList inline begin
void MemoryPool::ChunkList::AppendFreeList(MemoryPool::MemoryBlock* block) RASP_NOEXCEPT {
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


inline void MemoryPool::ChunkList::EraseFromDeallocedList(MemoryPool::MemoryBlock* find, MemoryPool::MemoryBlock* last) RASP_NOEXCEPT {
  if (last != nullptr) {
    last->set_next_ptr(find->ToNextPtr()->ToBegin());
  } else {
    free_head_ = find->ToNextPtr();
  }
  find->set_next_ptr(find->next_addr()->ToBegin());
}


RASP_INLINE MemoryPool::MemoryBlock* MemoryPool::ChunkList::SwapFreeHead() RASP_NOEXCEPT {
  MemoryBlock* block = free_head_;
  free_head_ = block->ToNextPtr();
  return block;
}
// ChunkList inline end


// CentralArena inline begin
inline MemoryPool::MemoryBlock* MemoryPool::CentralArena::Commit(size_t size, size_t default_size) {
  ASSERT(true, size > 0);
  int index = FindBestFitBlockIndex(size);
  ChunkList* chunk_list = InitChunk(size, default_size, index);

  if (chunk_list->free_head() != nullptr) {
    RASP_CHECK(true, index <= kMaxSmallObjectsCount);
    return chunk_list->SwapFreeHead();
  }
  
  return chunk_list->current()->GetBlock(size);
}


void MemoryPool::CentralArena::FreeArena(MemoryPool::LocalArena* arena) {
  //std::cout << "ReleaseLock addr: " << arena << " thread: " << std::this_thread::get_id() << std::endl;
  arena->ReleaseLock();
}


inline int MemoryPool::CentralArena::FindBestFitBlockIndex(size_t size) {
  RASP_CHECK(true, size > 0);
  return (size / kAlignment);
}


MemoryPool::LocalArena* MemoryPool::CentralArena::FindUnlockedArena() {
  LocalArena* arena = arena_head_;
  while (arena != nullptr) {
    if (arena->AcquireLock()) {
      //std::cout << "AcquireLock addr: "  << arena << " thread: " << std::this_thread::get_id() << std::endl;
      return arena;
    }
    arena = arena->next();
  }
  return nullptr;
}


inline MemoryPool::ChunkList* MemoryPool::CentralArena::InitChunk(
    size_t size, size_t default_size, int index) {
  ChunkList* chunk_list = TlsAlloc()->chunk_list(index);
  size_t heap_size = index == 0? default_size: RASP_ALIGN_OFFSET(((size + (kPointerSize * 2)) * 50), kAlignment);
  chunk_list->AllocChunkIfNecessary(size, heap_size, mmap_);
  return chunk_list;
}


MemoryPool::LocalArena* MemoryPool::CentralArena::TlsAlloc() {
  LocalArena* arena = nullptr;
  if (arena_head_ != nullptr) {
    arena = FindUnlockedArena();
  }

  if (arena == nullptr) {
    arena = reinterpret_cast<LocalArena*>(tls_->Get());
    if (arena == nullptr) {
      void* block = mmap_->Commit(sizeof(LocalArena));
      arena = new(block) LocalArena(this, mmap_, huge_chunk_allocator_);
      arena->AcquireLock();
      StoreNewLocalArena(arena);
      tls_->Set(arena);
      return arena;
    }
  }
  
  return arena;
}


void MemoryPool::CentralArena::StoreNewLocalArena(MemoryPool::LocalArena* arena) {
  ScopedSpinLock lock(tree_lock_);
  if (arena_head_ == nullptr) {
    arena_head_ = arena_tail_ = arena;
  } else {
    arena_tail_->set_next(arena);
    arena_tail_ = arena;
  }
}
// CentralArena inline end


// LocalArena inline begin
MemoryPool::LocalArena::LocalArena(
    MemoryPool::CentralArena* central_arena, Mmap* mmap, HugeChunkAllocator* huge_chunk_allocator)
    : central_arena_(central_arena),
      mmap_(mmap),
      huge_chunk_map_(*huge_chunk_allocator),
      next_(nullptr) {
  lock_.clear();
  classed_chunk_list_ = reinterpret_cast<ChunkList*>(mmap_->Commit(sizeof(ChunkList) * (kMaxSmallObjectsCount + 1)));
  for (int i = 0; i < kMaxSmallObjectsCount; i++) {
    new(classed_chunk_list_ + i) ChunkList();
  }
}


MemoryPool::LocalArena::~LocalArena() {
  for (int i = 0; i < kMaxSmallObjectsCount; i++) {
    (classed_chunk_list_ + i)->~ChunkList();
  }
}


void MemoryPool::LocalArena::Return() {
  central_arena_->FreeArena(this);
}
// LocalArena inline end

} // namespace rasp

#endif
