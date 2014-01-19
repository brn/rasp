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


inline void* Allocatable::operator new(size_t size, MemoryPool* pool) {
  return pool->AllocAllocatable(size);
}
inline void Allocatable::operator delete(void*) {}
inline void Allocatable::operator delete(void* ptr, MemoryPool*){ operator delete(ptr); }

static boost::thread_specific_ptr<MemoryPool> tls_;


// Create Chunk from byte block.
// Chunk and heap block is create from one big memory block.
// The structure is below
// |8-BIT VERIFY BIT|Chunk MEMORY BLOCK|The memory block managed BY Chunk|
RASP_INLINE MemoryPool::Chunk* MemoryPool::Chunk::New(size_t size) {
  static const size_t kChunkSize = RASP_ALIGN(sizeof(Chunk), kAlignment);
  const size_t aligned_size = RASP_ALIGN(size, kAlignment);
  
  // All heap size we want.
  const size_t heap_size = RASP_ALIGN((kVerificationTagSize + kChunkSize + aligned_size), kAlignment);
  Byte* ptr = new Byte[heap_size];

  // Verification bit.
  VerificationTag* tag = reinterpret_cast<VerificationTag*>(ptr);
  (*tag) = kVerificationBit;
  void* chunk_area = reinterpret_cast<void*>(ptr + kVerificationTagSize);

  // Instantiate Chunk from the memory block.
  return new(chunk_area) Chunk((reinterpret_cast<Byte*>(chunk_area) + kChunkSize), aligned_size);
}


RASP_INLINE void MemoryPool::Chunk::Delete(Chunk* chunk) {
  chunk->Destruct();
  chunk->~Chunk();
  Byte* block = reinterpret_cast<Byte*>(chunk);
  Byte* block_begin = block - kVerificationTagSize;
  VerificationTag* tag = reinterpret_cast<VerificationTag*>(block_begin);
  ASSERT(true, (*tag) == kVerificationBit);
  delete[] block_begin;
}


// Get heap from the pool.
// The heap structure is bellow
// |1-BIT SENTINEL-FLAG|1-BIT Allocatable FLAG|14-BIT SIZE BIT|FREE-MEMORY|
// This method return FREE-MEMORY area.
inline void* MemoryPool::Chunk::GetBlock(size_t reserve, bool is_allocatable) {
  ASSERT(true, HasEnoughSize(reserve));
  // unmark sentinel bit of the last reserved tag.
  unset_sentinel_bit();
  
  uint8_t* ret = (block_ + used_);
  size_t reserved_size = (reserve + kTagBitSize);
  if ((block_size_ - used_) >= reserved_size) {
    used_ += reserved_size;
    used_ = RASP_ALIGN(used_, kAlignment);
  }
  last_reserved_tag_ = reinterpret_cast<TagBit*>(ret);
  (*last_reserved_tag_) = reserve;

  // We mark as allocatable if is_allocatable flag is true.
  // Because, in Destruct method we call destructor of Allocatable
  // if the Allocatable bit is on.
  if (is_allocatable) {
    (*last_reserved_tag_) |= kAllocatableTagBit;
  }

  // mark as sentinel.
  set_sentinel_bit();
  return static_cast<void*>(ret + kTagBitSize);
}


RASP_INLINE void MemoryPool::Chunk::set_sentinel_bit() RASP_NOEXCEPT {
  if (last_reserved_tag_ != nullptr) {
    (*last_reserved_tag_) |= kSentinelBit;
  }
}


RASP_INLINE void MemoryPool::Chunk::unset_sentinel_bit() RASP_NOEXCEPT {
  if (last_reserved_tag_ != nullptr) {
    (*last_reserved_tag_) &= kMsbMask;
  }
}


//MemoryPool constructor.
inline MemoryPool::MemoryPool(size_t size)
    : size_(size),
      non_class_ptr_head_(nullptr),
      current_non_class_ptr_(nullptr),
      allocatable_head_(nullptr),
      current_allocatable_(nullptr),
      deleted_(false){
  ASSERT(true, size <= kMaxAllocatableSize);
  current_chunk_ = chunk_head_ = new SinglyLinkedList<MemoryPool::Chunk>(MemoryPool::Chunk::New(size_));
}



template <typename T, typename Deleter>
void MemoryPool::Delete(T head, Deleter d) {
  if (head != nullptr) {
    auto v = head;
    while (v != nullptr) {
      auto alloc = v->value();
      ASSERT(true, alloc != nullptr);
#ifdef UNIT_TEST
      ReserveForTest(alloc);
#endif
      d(alloc);
      auto tmp = v;
      v = v->next();
      delete tmp;
    }
  }
  head = nullptr;
}



inline MemoryPool* MemoryPool::local_instance(size_t size) {
  if (tls_.get() == NULL) {
    tls_.reset(new MemoryPool(size));
  }
  return tls_.get();
}


template <typename T>
inline T* MemoryPool::Allocate(size_t size) {
  return reinterpret_cast<T*>(Alloc<T>(size, false));
}


//Allocate chunk.
inline void* MemoryPool::AllocAllocatable(size_t size) {
  return Alloc<Allocatable>(size, true);
}


template <typename T>
inline void* MemoryPool::Alloc(size_t size, bool is_allocatable) {
  size_t aligned = RASP_ALIGN(size, kAlignment);
  T* block = nullptr;
  if (aligned <= size_) {
    if (!current_chunk_->value()->HasEnoughSize(aligned)) {      
      current_chunk_->set_next(new SinglyLinkedList<MemoryPool::Chunk>(MemoryPool::Chunk::New(size_)));
      current_chunk_ = current_chunk_->next();
    }
    if (is_allocatable) {
      static const size_t kClassTypeSize = sizeof(T);
      if (size < kClassTypeSize) {
        size = kClassTypeSize;
      }
    }
    block = reinterpret_cast<T*>(current_chunk_->value()->GetBlock(aligned, is_allocatable));
  } else {
    block = reinterpret_cast<T*>(malloc(size));
    if (block == NULL) {
      throw std::bad_alloc();
    }
    Append(block);
  }
  
  return static_cast<void*>(block);
}


template <typename T>
inline void MemoryPool::Append(T* pointer)  {
  auto allocated_list = new SinglyLinkedList<std::remove_pointer<T>::type>();
  if (allocatable_head_ == nullptr) {
    allocatable_head_ = current_allocatable_ = allocated_list;
  } else {
    ASSERT(true, current_allocatable_ != nullptr);
    current_allocatable_->set_next(allocated_list);
    current_allocatable_ = allocated_list;
  }
  current_allocatable_->set_value(pointer);
}

} // namespace rasp



#endif
