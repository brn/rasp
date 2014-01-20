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
  void* chunk_area = PtrAdd(ptr, kVerificationTagSize);

  // Instantiate Chunk from the memory block.
  return new(chunk_area) Chunk(reinterpret_cast<Byte*>(PtrAdd(chunk_area, kChunkSize)), aligned_size);
}


RASP_INLINE void MemoryPool::Chunk::Delete(Chunk* chunk) RASP_NOEXCEPT {
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
inline void* MemoryPool::Chunk::GetBlock(size_t reserve) RASP_NOEXCEPT  {
  ASSERT(true, HasEnoughSize(reserve));
  // unmark sentinel bit of the last reserved tag.
  unset_sentinel_bit();
  Byte* ret = block_ + used_;
  size_t offset = kTagBitSize + kDisposableBaseSize;
  size_t reserved_size = RASP_ALIGN((offset + reserve), kAlignment);

  used_ += reserved_size;
  last_reserved_tag_ = reinterpret_cast<TagBit*>(ret);
  (*last_reserved_tag_) = reserve;
  ret += kTagBitSize;

  // mark as sentinel.
  set_sentinel_bit();
  return static_cast<void*>(ret);
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
    : size_(size + (size / kPointerSize) * 2),
      malloced_head_(nullptr),
      current_malloced_(nullptr),
      deleted_(false){
  ASSERT(true, size <= kMaxAllocatableSize);
  current_chunk_ = chunk_head_ = new SinglyLinkedList<MemoryPool::Chunk>(MemoryPool::Chunk::New(size_));
}



template <typename T, typename Deleter>
void MemoryPool::Delete(T head, Deleter d) RASP_NOEXCEPT {
  if (head != nullptr) {
    auto v = head;
    while (v != nullptr) {
      auto alloc = v->value();
      ASSERT(true, alloc != nullptr);
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


template <typename T, typename ... Args>
RASP_INLINE T* MemoryPool::Allocate(Args ... args) {
  static_assert(std::is_destructible<T>::value == true,
                "The allocatable type of MemoryPool::Allocate must be a destructible type.");
  return new(Alloc<std::remove_const<T>::type,
             std::is_class<T>::value && !std::is_enum<T>::value>(sizeof(T))) T(args...);
}


template <typename T>
RASP_INLINE T* MemoryPool::AllocateArray(size_t size) {
  static_assert(std::is_destructible<T>::value == true,
                "The allocatable type of MemoryPool::Allocate must be a destructible type.");
  ASSERT(true, size > 0u);
  return new(Alloc<std::remove_const<T>::type,
             std::is_class<T>::value && !std::is_enum<T>::value>(RASP_ALIGN((sizeof(T) * size), kAlignment))) T[size];
}


template <typename T, bool is_class_type>
void* MemoryPool::Alloc(size_t size) {
  if (dealloced_list_.size() > 0) {
    void* block = ReAllocate<T, is_class_type>(size);
    if (block != nullptr) {
      return block;
    }
  }
  if (size <= size_) {
    return AllocFromChunk<T, is_class_type>(size);
  }
  return AllocFromHeap<T, is_class_type>(size);
}


template <typename T, bool is_class_type>
inline void* MemoryPool::ReAllocate(size_t size) {
  DeallocedList::iterator it = dealloced_list_.begin();
  DeallocedList::iterator end = dealloced_list_.end();
  DeallocedList::iterator find = FindApproximateDeallocedBlock(size, it, end);
    
  if (find != end) {
    Byte* block_begin = *find;
    void* block = reinterpret_cast<void*>(block_begin);
    TagBit* bit = reinterpret_cast<TagBit*>(block);
    (*bit) &= kDeallocedBitMask;
    block = PtrAdd(block, kTagBitSize);
    DisposableBase* base = reinterpret_cast<DisposableBase*>(block);
    bool malloced = base->IsMalloced();
    base->~DisposableBase();
    if (malloced) {
      new(block) Disposable<T, true, is_class_type>();
    } else {
      new(block) Disposable<T, false, is_class_type>();
    }
    block = PtrAdd(block, kDisposableBaseSize);
    dealloced_list_.erase(find);
    return static_cast<void*>(block);
  }
  return nullptr;
}



template <typename T, bool is_class_type>
inline void* MemoryPool::AllocFromChunk(size_t size) {
  void* block = nullptr;
  AllocChunkIfNecessary(size);
    
  if (is_class_type) {
    const size_t kClassTypeSize = sizeof(T);
    if (size < kClassTypeSize) {
      size = kClassTypeSize;
    }
  }
  block = current_chunk_->value()->GetBlock(size);
  new (block) Disposable<T, false, is_class_type>();
  return PtrAdd(block ,kDisposableBaseSize);
}


template <typename T, bool is_class_type>
inline void* MemoryPool::AllocFromHeap(size_t size) {
  void* block = nullptr;
  void* block_begin = block = malloc(RASP_ALIGN((kTagBitSize + kDisposableBaseSize + size), kAlignment));
  if (block_begin == NULL) {
    throw std::bad_alloc();
  }
  TagBit* bit = reinterpret_cast<TagBit*>(block);
  (*bit) = 0;
  block = PtrAdd(block, kTagBitSize);
  new (block) Disposable<T, true, is_class_type>();
  block = PtrAdd(block, kDisposableBaseSize);
  Append(block_begin);
  return block;
}


inline void MemoryPool::Append(void* pointer)  {
  auto allocated_list = new SinglyLinkedList<void>();
  if (malloced_head_ == nullptr) {
    malloced_head_ = current_malloced_ = allocated_list;
  } else {
    ASSERT(true, current_malloced_ != nullptr);
    current_malloced_->set_next(allocated_list);
    current_malloced_ = allocated_list;
  }
  current_malloced_->set_value(pointer);
}


RASP_INLINE MemoryPool::DeallocedList::iterator MemoryPool::FindApproximateDeallocedBlock(
    size_t size,
    MemoryPool::DeallocedList::iterator& begin,
    MemoryPool::DeallocedList::iterator& end) {
  DeallocedList::iterator find = end;
  uint16_t most = UINT16_MAX;
  for (;begin != end; ++begin) {
    uint16_t block_size = ((*reinterpret_cast<TagBit*>(*begin)) & kFlagMask);
    if (block_size == size) {
      return begin;
    }
    if (block_size >= size) {
      uint16_t s = block_size - size;
      if (most > s) {
        most = s;
        find = begin;
      }
    }
  }
  if (most > (kTagBitSize + kDisposableBaseSize + kPointerSize) && find != end) {
    Byte* split = (*find) + kTagBitSize + kDisposableBaseSize + size;
    TagBit* bit = reinterpret_cast<TagBit*>(split);
    (*bit) = kDeallocedBit;
    dealloced_list_.push_back(split);
  }
  return find;
}

} // namespace rasp



#endif
