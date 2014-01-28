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
    Pointer p = reinterpret_cast<Pointer>(ToBegin() + kValueOffset);
    p |= kDeallocedBit;
  }


  RASP_INLINE void UnmarkDealloced() RASP_NOEXCEPT {
    Pointer p = reinterpret_cast<Pointer>(ToBegin() + kValueOffset);
    p &= kDeallocedMask;
  }


  RASP_INLINE bool IsMarkedAsDealloced() RASP_NOEXCEPT {
    return (reinterpret_cast<Pointer>(ToBegin() + kValueOffset) & kDeallocedBit) == kDeallocedBit;
  }


  RASP_INLINE Byte* ToBegin() RASP_NOEXCEPT {
    return reinterpret_cast<Byte*>(this);
  }
};


inline void* Poolable::operator new(size_t size, MemoryPool* pool) {
  return pool->DistributeBlockWhileLocked(size);
}


inline void* Poolable::operator new[](size_t size, MemoryPool* pool) {
  return pool->DistributeBlockWhileLocked(size);
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
inline rasp::MemoryPool::MemoryBlock* MemoryPool::Chunk::GetBlock(size_t reserve, MemoryPool* pool) RASP_NOEXCEPT  {
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
    : chunk_bundle_(new ChunkBundle()),
      dealloced_head_(nullptr),
      current_dealloced_(nullptr),
      size_(size + (size / kPointerSize) * 2),
      deleted_(false) {
  ASSERT(true, size <= kMaxAllocatableSize);
}



inline MemoryPool* MemoryPool::local_instance(size_t size) {
  if (tls_.get() == NULL) {
    tls_.reset(new MemoryPool(size));
  }
  return tls_.get();
}


RASP_INLINE void* MemoryPool::Allocate(size_t size) {
  return DistributeBlockWhileLocked(size);
}


inline void MemoryPool::Dealloc(void* object) {
  //std::lock_guard<std::mutex> lock(deallocation_mutex_);
  
  Byte* block = reinterpret_cast<Byte*>(object);
  block -= MemoryPool::kValueOffset;
  MemoryPool::MemoryBlock* memory_block = reinterpret_cast<MemoryPool::MemoryBlock*>(block);
  
  if (!memory_block->IsMarkedAsDealloced()) {
    memory_block->ToValue()->~Poolable();
    memory_block->MarkAsDealloced();
    if (dealloced_head_ == nullptr) {
      current_dealloced_ = dealloced_head_ = memory_block;
    } else {
      current_dealloced_->set_next_ptr(memory_block->ToBegin());
    }
  }
}


inline void* MemoryPool::DistributeBlockWhileLocked(size_t size) {
  //std::lock_guard<std::mutex> lock(allocation_mutex_);
  
  size_t aligned_size = RASP_ALIGN_OFFSET(size, kAlignment);
  if (dealloced_head_ != nullptr) {
    void* block = DistributeBlockFromDeallocedList(aligned_size);
    if (block != nullptr) {
      return block;
    }
  }
  
  return DistributeBlockFromChunk(aligned_size);
}


inline void* MemoryPool::DistributeBlockFromDeallocedList(size_t size) {
  MemoryBlock* memory_block = FindApproximateDeallocedBlock(size);
    
  if (memory_block != nullptr) {
    memory_block->UnmarkDealloced();
    return memory_block->ToValue<void>();
  }
  return nullptr;
}


inline void* MemoryPool::DistributeBlockFromChunk(size_t size) {
  const size_t kPoolableSize = sizeof(Poolable);
  Chunk* chunk = chunk_bundle_->chunk(size, size_, &allocator_);
  MemoryBlock* block = nullptr;
    
  if (kPoolableSize > size) {
    size = kPoolableSize;
  }
  
  block = chunk->GetBlock(size, this);
  return block->ToValue<void>();
}


inline void MemoryPool::EraseFromDeallocedList(MemoryPool::MemoryBlock* find, MemoryPool::MemoryBlock* last) {
  if (last != nullptr) {
    last->set_next_ptr(find->ToNextPtr()->ToBegin());
  } else {
    dealloced_head_ = find->ToNextPtr();
  }
  find->set_next_ptr(find->next_addr()->ToBegin());
}


inline MemoryPool::MemoryBlock* MemoryPool::FindApproximateDeallocedBlock(size_t size) {
  MemoryBlock* last = nullptr;
  MemoryBlock* find = nullptr;
  MemoryBlock* current = dealloced_head_;
  Size most = kMaxAllocatableSize;
  
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



inline MemoryPool::Chunk* MemoryPool::ChunkBundle::chunk(size_t size, size_t default_size, Mmap* mmap) {
  ASSERT(true, size > 0);
  int index;
  if (size < kAlignment) {
    index = 0;
  } else {
    index = size / kAlignment;
  }
  
  if (index > 9) {
    index = 0;
  }
  return InitChunk(size, index == 0? default_size : size * 500, index, mmap);
}


inline void MemoryPool::ChunkBundle::Destroy() {
  for (int i = 0; i < 10; i++) {
    auto chunk_list = bundles_[i];
    if (chunk_list.head() != nullptr) {
      auto chunk = chunk_list.head();
      while (chunk != nullptr) {
        ASSERT(true, chunk != nullptr);
        auto tmp = chunk;
        chunk = chunk->next();
        Chunk::Delete(tmp);
      }
    }
  }
}


inline MemoryPool::Chunk* MemoryPool::ChunkBundle::InitChunk(size_t size, size_t default_size, int index, Mmap* mmap) {
  ChunkList* chunk_list = &bundles_[index];
  chunk_list->AllocChunkIfNecessary(size, default_size, mmap);
  return chunk_list->current();
}


inline void MemoryPool::ChunkBundle::ChunkList::AllocChunkIfNecessary(size_t size, size_t default_size, Mmap* mmap) {
  if (head_ == nullptr) {
    current_ = head_ = MemoryPool::Chunk::New(default_size, mmap);
  }
    
  if (!current_->HasEnoughSize(size)) {
    size_t needs = size > default_size? size + kValueOffset: default_size;
    current_->set_next(MemoryPool::Chunk::New(needs, mmap));
    current_ = current_->next();
  }
}

} // namespace rasp

#endif
