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

class Regions::Header {
 public:
  RASP_INLINE size_t size() RASP_NOEXCEPT {
    return (*reinterpret_cast<size_t*>(ToBegin())) & kTagRemoveBit;
  }


  RASP_INLINE void set_size(size_t size) RASP_NOEXCEPT {
    size_t* size_bit = reinterpret_cast<size_t*>(ToBegin());
    *size_bit = size;
  }
  

  RASP_INLINE Byte* ToBegin() RASP_NOEXCEPT {
    return reinterpret_cast<Byte*>(this);
  }


  RASP_INLINE Regions::FreeHeader* ToFreeHeader() RASP_NOEXCEPT {
    return reinterpret_cast<FreeHeader*>(reinterpret_cast<Byte*>(this) + kHeaderSize);
  }


  RASP_INLINE Header* next_addr() RASP_NOEXCEPT {
    return reinterpret_cast<Header*>(ToValue<Byte*>() + size());
  }


  template <typename T = RegionalObject*>
  RASP_INLINE typename std::remove_pointer<T>::type* ToValue() RASP_NOEXCEPT {
    return reinterpret_cast<typename std::remove_pointer<T>::type*>(ToBegin() + kSizeTSize);
  }


  RASP_INLINE void MarkAsDealloced() RASP_NOEXCEPT {
    size_ |= kDeallocedBit;
  }


  RASP_INLINE void UnmarkDealloced() RASP_NOEXCEPT {
    size_ &= kDeallocedMask;
  }


  RASP_INLINE bool IsMarkedAsDealloced() RASP_NOEXCEPT {
    return (size_ & kDeallocedBit) == kDeallocedBit;
  }


  RASP_INLINE void MarkAsArray() RASP_NOEXCEPT {
    size_ |= kArrayBit;
  }


  RASP_INLINE bool IsMarkedAsArray() RASP_NOEXCEPT {
    return (size_ & kArrayBit) == kArrayBit;
  }

 private:
  size_t size_;
};


class Regions::FreeHeader {
 public:    

  RASP_INLINE FreeHeader* ToNextPtr() RASP_NOEXCEPT {
    return reinterpret_cast<FreeHeader*>(next_);
  }


  RASP_INLINE void set_next_ptr(Byte* next_ptr) RASP_NOEXCEPT {
    next_ = next_ptr;
  }


  RASP_INLINE Byte* ToBegin() RASP_NOEXCEPT {
    return reinterpret_cast<Byte*>(this);
  }


  RASP_INLINE Regions::Header* ToHeader() RASP_NOEXCEPT {
    return reinterpret_cast<Header*>(ToBegin() - kFreeHeaderSize);
  }

  
 private:
  Byte* next_;
};



inline void* RegionalObject::operator new(size_t size, Regions* pool) {
  return pool->Allocate(size);
}


inline void* RegionalObject::operator new[](size_t size, Regions* pool) {
  return pool->AllocateArray(size);
}


void Regions::Destroy() RASP_NOEXCEPT {
  if (!deleted_.test_and_set()) {
    central_arena_->Destroy();
  }
}



void Regions::Dealloc(void* object) RASP_NOEXCEPT {
  central_arena_->Dealloc(object);
}


void* Regions::Allocate(size_t size) {
  return DistributeBlock(size)->ToValue<void>();
}


void* Regions::AllocateArray(size_t size) {
  Header* header = DistributeBlock(size);
  header->MarkAsArray();
  return header->ToValue<void>();
}


void Regions::DestructRegionalObject(Regions::Header* header) {
  if (!header->IsMarkedAsArray()) {
    header->ToValue()->~RegionalObject();
  } else {
    delete[] header->ToValue();
  }
}


Regions::Header* Regions::DistributeBlock(size_t size) {
  const size_t kRegionalObjectSize = sizeof(RegionalObject);

  if (kRegionalObjectSize > size) {
    size = kRegionalObjectSize;
  }
  return central_arena_->Commit(size, size_);
}


// Create Chunk from byte block.
// Chunk and heap block is create from one big memory block.
// The structure is below
// |8-BIT VERIFY BIT|Chunk MEMORY BLOCK|The memory block managed BY Chunk|
Regions::Chunk* Regions::Chunk::New(size_t size, Mmap* allocator) {
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


void Regions::Chunk::Delete(Chunk* chunk) RASP_NOEXCEPT {
  chunk->Destruct();
  chunk->~Chunk();
}


bool Regions::Chunk::HasEnoughSize(size_t needs) RASP_NO_SE {
  needs = needs > kFreeHeaderSize? needs: kFreeHeaderSize;
  return block_size_ >= used_ + (RASP_ALIGN_OFFSET((kValueOffset + needs), kAlignment));
}
// Chunk inline end


// ChunkList inline begin
inline Regions::Header* Regions::ChunkList::AllocChunkIfNecessary(size_t size, Mmap* mmap, Regions::CentralArena* arena) {
  if (head_ == nullptr) {
    current_ = head_ = Regions::Chunk::New(RASP_ALIGN_OFFSET((100 KB), kAlignment), mmap);
  }
  
  if (!current_->HasEnoughSize(size)) {
    Header* header = arena->FindFreeChunk(size);
    if (header != nullptr) {
      return header;
    }
    current_->set_next(Regions::Chunk::New(RASP_ALIGN_OFFSET((100 KB), kAlignment), mmap));
    current_ = current_->next();
  }
  return current_->GetBlock(size);
}
// ChunkList inline end


// CentralArena inline begin
inline Regions::Header* Regions::CentralArena::Commit(size_t size, size_t default_size) {
  ASSERT(true, size > 0);
  int index = FindBestFitBlockIndex(size);
  LocalArena* local_arena = TlsAlloc();

  if (local_arena->has_free_chunk(index)) {
    return local_arena->free_chunk_stack(index)->Shift();
  }

  ChunkList* chunk_list = local_arena->chunk_list();
  return chunk_list->AllocChunkIfNecessary(size, local_arena->allocator(), this);
}


void Regions::CentralArena::FreeArena(Regions::LocalArena* arena) {
  FreeChunkStack* free_chunk_stack = arena->free_chunk_stack();
  for (int i = 0; i < kMaxSmallObjectsCount; i++) {
    FreeHeader* free_header = free_chunk_stack[i].head();
    FreeChunkStack* central_free_chunk = &(central_free_chunk_stack_[i]);
    while (free_header != nullptr) {
      FreeHeader* next = free_header->ToNextPtr();
      central_free_chunk->Unshift(free_header->ToHeader());
      free_header = next;
    }
    free_chunk_stack[i].Clear();
  }

  FreeMap(arena);
  arena->ReleaseLock();
}


void Regions::CentralArena::FreeMap(Regions::LocalArena* arena) {
  ScopedSpinLock lock_(map_lock_);
  HugeChunkMap* map = arena->huge_free_chunk_map();
  huge_free_chunk_map_->insert(map->begin(), map->end());
  map->clear();
}


Regions::Header* Regions::CentralArena::FindFreeChunk(size_t size) {
  int index = FindBestFitBlockIndex(size);
  ScopedSpinLock lock(free_lock_);
  Header* ret = nullptr;
  
  if (index < kMaxSmallObjectsCount && central_free_chunk_stack_[index].HasHead()) {
    ret = central_free_chunk_stack_[index].Shift();
  } else if (huge_free_chunk_map_->count(index) > 0) {
    ret = huge_free_chunk_map_->operator[](index)->Shift();
  }

  return ret;
};


inline int Regions::CentralArena::FindBestFitBlockIndex(size_t size) RASP_NOEXCEPT {
  RASP_CHECK(true, size > 0);
  return (size / kAlignment) - 1;
}


Regions::LocalArena* Regions::CentralArena::FindUnlockedArena() RASP_NOEXCEPT {
  LocalArena* arena = arena_head_;
  while (arena != nullptr) {
    if (arena->AcquireLock()) {
      //printf("AcquireLock addr: %p thread: %zu\n", arena, std::hash<std::thread::id>()(std::this_thread::get_id()));
      return arena;
    }
    arena = arena->next();
  }
  return nullptr;
}


Regions::LocalArena* Regions::CentralArena::TlsAlloc() {
  LocalArena* arena = nullptr;
  
  if (arena == nullptr) {
    arena = reinterpret_cast<LocalArena*>(tls_->Get());
    if (arena == nullptr) {
      if (arena_head_ != nullptr) {
        arena = FindUnlockedArena();
      }
      if (arena == nullptr) {
        void* block = mmap_->Commit(sizeof(LocalArena));
        arena = new(block) LocalArena(this, huge_chunk_allocator_);
        arena->AcquireLock();
        StoreNewLocalArena(arena);
        tls_->Set(arena);
        return arena;
      }
    }
  }
  
  return arena;
}


void Regions::CentralArena::StoreNewLocalArena(Regions::LocalArena* arena) RASP_NOEXCEPT {
  ScopedSpinLock lock(tree_lock_);
  if (arena_head_ == nullptr) {
    arena_head_ = arena_tail_ = arena;
  } else {
    arena_tail_->set_next(arena);
    arena_tail_ = arena;
  }
}
// CentralArena inline end


// FreeChunkStack inline begin
void Regions::FreeChunkStack::Unshift(Regions::Header* header) RASP_NOEXCEPT {
  ScopedSpinLock lock(tree_lock_);
  ASSERT(true, header->IsMarkedAsDealloced());
  FreeHeader* block = header->ToFreeHeader();
  if (free_head_ == nullptr) {
    free_head_ = block;
    free_head_->set_next_ptr(nullptr);
  } else {
    block->set_next_ptr(free_head_->ToBegin());
    free_head_ = block;
  }
  ASSERT(true, free_head_->ToHeader()->IsMarkedAsDealloced());
}


RASP_INLINE Regions::Header* Regions::FreeChunkStack::Shift() RASP_NOEXCEPT {
  ScopedSpinLock lock(tree_lock_);
  Header* header = free_head_->ToHeader();
  ASSERT(true, header->IsMarkedAsDealloced());
  free_head_ = free_head_->ToNextPtr();
  header->UnmarkDealloced();
  ASSERT(false, header->IsMarkedAsDealloced());
  return header;
}
// FreeChunkStack inline end


// LocalArena inline begin
Regions::LocalArena::LocalArena(
    Regions::CentralArena* central_arena, HugeChunkAllocator* huge_chunk_allocator)
    : central_arena_(central_arena),
      huge_free_chunk_map_(*huge_chunk_allocator),
      next_(nullptr) {
  lock_.clear();
}


Regions::LocalArena::~LocalArena() {}


void Regions::LocalArena::Return() {
  central_arena_->FreeArena(this);
}
// LocalArena inline end

} // namespace rasp

#endif
