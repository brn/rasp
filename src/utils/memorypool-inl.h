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



RASP_INLINE MemoryPool::DisposableBase::DisposableBase(size_t array_size)
    : array_size_(array_size) {};


RASP_INLINE void MemoryPool::DisposableBase::Dispose(void* block_begin, void* ptr, bool is_free) const {
  DisposeInternal(block_begin, ptr, is_free);
}


template <typename T, bool kIsClassType, bool is_array>
class MemoryPool::Disposable : public MemoryPool::DisposableBase {};


template <typename T, bool is_array>
class MemoryPool::Disposable<T, true, is_array> : public MemoryPool::DisposableBase {
 public:
  explicit Disposable(size_t array_size)
      : DisposableBase(array_size){}
 private:
  RASP_INLINE virtual void DisposeInternal(void* block_begin, void* ptr, bool is_free) RASP_NO_SE {
    T* object = reinterpret_cast<T*>(ptr);
    if (!std::is_pod<T>::value) {
      if (!is_array) {
        object->~T();
        Pointer p = reinterpret_cast<Pointer>(ptr);
        p = kInvalidPointer;
      } else {
        for (size_t i = 0u; i < array_size_; i++) {
          object[i].~T();
          Pointer p = reinterpret_cast<Pointer>(ptr);
          p = kInvalidPointer;
        }
      }
    }
  }
};


template <typename T, bool is_array>
class MemoryPool::Disposable<T, false, is_array> : public MemoryPool::DisposableBase {
 public:
  Disposable(size_t array_size)
      : DisposableBase(array_size){}
 private:
  RASP_INLINE virtual void DisposeInternal(void* block_begin, void* ptr, bool is_free) RASP_NO_SE {
    Pointer p = reinterpret_cast<Pointer>(ptr);
    p = kInvalidPointer;
  }
};


struct MemoryPool::MemoryBlock {

  RASP_INLINE Size size() RASP_NO_SE {
    return *(ToSizeBit());
  }
    
    
  RASP_INLINE SizeBit* ToSizeBit() RASP_NO_SE {
    return reinterpret_cast<SizeBit*>(ToBegin());
  }


  RASP_INLINE MemoryBlock* next_addr() RASP_NO_SE {
    return reinterpret_cast<MemoryBlock*>(ToValue() + size());
  }
    

  RASP_INLINE MemoryBlock* ToNextPtr() RASP_NO_SE {
    return reinterpret_cast<MemoryBlock*>(*(reinterpret_cast<Byte**>(ToBegin() + kSizeBitSize)));
  }

    
  RASP_INLINE void set_next_ptr(Byte* next_ptr) RASP_NO_SE {
    Byte** next_head = reinterpret_cast<Byte**>(ToBegin() + kSizeBitSize);
    *next_head = next_ptr;
  }


  RASP_INLINE void set_next_ptr(MemoryBlock* next_ptr) RASP_NO_SE {
    Byte** next_head = reinterpret_cast<Byte**>(ToBegin() + kSizeBitSize);
    *next_head = next_ptr->ToBegin();
  }
    

  template <typename T = DisposableBase*>
  RASP_INLINE typename std::remove_pointer<T>::type* ToDisposable() RASP_NO_SE {
    Pointer p = reinterpret_cast<Pointer>(ToBegin() + kDisposableOffset);
    return reinterpret_cast<typename std::remove_pointer<T>::type*>(p & kTagRemoveBit);
  }


  template <typename T = Byte*>
  RASP_INLINE typename std::remove_pointer<T>::type* ToValue() RASP_NO_SE {
    return reinterpret_cast<typename std::remove_pointer<T>::type*>(ToBegin() + kValueOffset);
  }


  RASP_INLINE void MarkAsDealloced() RASP_NOEXCEPT {
    Pointer p = reinterpret_cast<Pointer>(ToBegin() + kSizeBitSize);
    p |= kDeallocedBit;
  }


  RASP_INLINE void UnmarkDealloced() RASP_NOEXCEPT {
    Pointer p = reinterpret_cast<Pointer>(ToBegin() + kSizeBitSize);
    p &= kDeallocedMask;
  }


  RASP_INLINE bool IsMarkedAsDealloced() RASP_NO_SE {
    return (reinterpret_cast<Pointer>(ToBegin() + kSizeBitSize) & kDeallocedBit) == kDeallocedBit;
  }


  RASP_INLINE Byte* ToBegin() RASP_NO_SE {
    return reinterpret_cast<Byte*>(const_cast<MemoryBlock*>(this));
  }
};


static boost::thread_specific_ptr<MemoryPool> tls_;


inline void MemoryPool::Chunk::Delete(Chunk* chunk, HeapAllocator* allocator) RASP_NOEXCEPT {
  chunk->Destruct();
  chunk->~Chunk();
  Byte* block = reinterpret_cast<Byte*>(chunk);
  Byte* block_begin = block - kVerificationTagSize;
  VerificationTag* tag = reinterpret_cast<VerificationTag*>(block_begin);
  ASSERT(true, (*tag) == kVerificationBit);
  allocator->Deallocate(block_begin);
}


RASP_INLINE bool MemoryPool::Chunk::HasEnoughSize(size_t needs) RASP_NO_SE {
  return block_size_ >= used_ + kValueOffset + needs;
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
  
  size_t reserved_size = RASP_ALIGN((kValueOffset + reserve), kAlignment);

  used_ += reserved_size;
  tail_block_ = ret;

  SizeBit* bit = reinterpret_cast<SizeBit*>(ret);
  (*bit) = reserve;
  Byte** next_ptr = reinterpret_cast<Byte**>(ret + kSizeBitSize);
  next_ptr = nullptr;

  return reinterpret_cast<MemoryBlock*>(ret);
}


//MemoryPool constructor.
inline MemoryPool::MemoryPool(size_t size)
    : size_(size + (size / kPointerSize) * 2),
      deleted_(false),
      current_chunk_(nullptr),
      chunk_head_(nullptr),
      dealloced_head_(nullptr),
      current_dealloced_(nullptr) {
  ASSERT(true, size <= kMaxAllocatableSize);
}


inline uint64_t MemoryPool::CommitedSize() RASP_NO_SE {
  Chunk* c = chunk_head_;
  uint64_t size = 0;
  while (c != nullptr) {size += c->size();c = c->next();}
  return size;
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
  return new(Alloc<typename std::remove_const<T>::type,
             std::is_class<T>::value && !std::is_enum<T>::value, false>(sizeof(T))) T(args...);
}


template <typename T>
RASP_INLINE T* MemoryPool::AllocateArray(size_t size) {
  static_assert(std::is_destructible<T>::value == true,
                "The allocatable type of MemoryPool::AllocateArray must be a destructible type.");
  ASSERT(true, size > 0u);
  static const bool is_class_type = std::is_class<T>::value && !std::is_enum<T>::value;
  static const size_t kSize = sizeof(T);
  T* ptr = reinterpret_cast<T*>(Alloc<typename std::remove_const<T>::type,
                                is_class_type, true>(kSize * size, size));
  T* head = ptr;
  if (is_class_type) {
    for (size_t i = 0u; i < size; i++) {
      new(reinterpret_cast<void*>(ptr)) T();
      ++ptr;
    }
  }
  return head;
}


inline void MemoryPool::Dealloc(void* object) {
  Byte* block = reinterpret_cast<Byte*>(object);
  block -= kValueOffset;
  MemoryBlock* memory_block = reinterpret_cast<MemoryBlock*>(block);
  memory_block->ToDisposable()->Dispose(
      memory_block->ToSizeBit(), memory_block->ToValue<void>(), false);
  memory_block->MarkAsDealloced();
  if (dealloced_head_ == nullptr) {
    current_dealloced_ = dealloced_head_ = memory_block;
  } else {
    current_dealloced_->set_next_ptr(memory_block->ToBegin());
  }
}


template <typename T, bool is_class_type, bool is_array>
inline void* MemoryPool::Alloc(size_t size, size_t array_size) {
  size_t aligned_size = RASP_ALIGN(size, kAlignment);
  if (dealloced_head_ != nullptr) {
    void* block = ReAllocate<T, is_class_type, is_array>(aligned_size, array_size);
    if (block != nullptr) {
      return block;
    }
  }
  
  return AllocFromChunk<T, is_class_type, is_array>(aligned_size, array_size);
}


template <typename T, bool is_class_type, bool is_array>
inline void* MemoryPool::ReAllocate(size_t size, size_t array_size) {
  MemoryBlock* memory_block = FindApproximateDeallocedBlock(size);
    
  if (memory_block != nullptr) {
    memory_block->UnmarkDealloced();
    memory_block->ToDisposable()->~DisposableBase();
    new(memory_block->ToDisposable<void>()) Disposable<T, is_class_type, is_array>(array_size);
    return memory_block->ToValue<void>();
  }
  return nullptr;
}



template <typename T, bool is_class_type, bool is_array>
inline void* MemoryPool::AllocFromChunk(size_t size, size_t array_size) {
  AllocChunkIfNecessary(size);
  MemoryBlock* block = nullptr;
    
  if (is_class_type) {
    const size_t kClassTypeSize = sizeof(T);
    if (size < kClassTypeSize) {
      size = kClassTypeSize;
    }
  }
  block = current_chunk_->GetBlock(size);
  new (block->ToDisposable<void>()) Disposable<T, is_class_type, is_array>(array_size);
  return block->ToValue<void>();
}


inline void MemoryPool::EraseFromDeallocedList(MemoryPool::MemoryBlock* find, MemoryPool::MemoryBlock* last) {
  if (last != nullptr) {
    last->set_next_ptr(find->ToNextPtr());
  } else {
    dealloced_head_ = find->ToNextPtr();
  }
  find->set_next_ptr(find->next_addr());
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


inline void MemoryPool::AllocChunkIfNecessary(size_t size) {
  if (chunk_head_ == nullptr) {
    current_chunk_ = chunk_head_ = MemoryPool::Chunk::New(size_, &allocator_);
  }
    
  if (!current_chunk_->HasEnoughSize(size)) {
    size_t needs = size > size_? size + kValueOffset: size_;
    current_chunk_->set_next(MemoryPool::Chunk::New(needs, &allocator_));
    current_chunk_ = current_chunk_->next();
  }
}

} // namespace rasp

#endif
