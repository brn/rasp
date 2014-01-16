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


namespace rasp {


static const size_t kAlignment = sizeof(void*);
static const size_t kAllocatableSize = MemoryPool::Align(sizeof(Allocatable), kAlignment);
static const size_t kTagSize = MemoryPool::Align(sizeof(uint8_t), kAlignment);


inline void* Allocatable::operator new(size_t size, MemoryPool* pool) {
  return pool->AllocClassType(size);
}
inline void Allocatable::operator delete(void*) {}
inline void Allocatable::operator delete(void* ptr, MemoryPool*){ operator delete(ptr); }



//MemoryPool constructor.
template <typename AllocatableSize>
inline MemoryPool<AllocatableSize>::MemoryPool()
    : current_(0),
      head_(0),
      head_block_(NULL),
      current_block_(NULL) {
  head_chunk_ = new Chunk<AllocatableSize>;
  current_chunk_ = head_chunk_;
}



template <typename AllocatableSize>
template <bool is_class_type>
template <typename T>
MemoryPool<AllocatableSize>::Deleter<is_class_type>::Delete(
    MemoryPool<AllocatableSize>::SinglyLinkedList<T>* head,
    MemoryPool<AllocatableSize>::SinglyLinkedList<T>* list) {
  
  if (head != nullptr) {
    auto v = head;
    while (v != nullptr) {
      T* alloc = v.value();
      ASSERT(true, alloc != nullptr);
      if (is_class_type) {
        alloc->~T();
      }
      free(alloc);
      auto tmp = v;
      v = v->next();
      delete tmp;
    }
  }
  head = nullptr;
}


template <typename AllocatableSize>
template <typename T>
inline T* MemoryPool<AllocatableSize>::Alloc(size_t size) {
  return reinterpret_cast<T*>(AllocateBlock(sizeof(T) * size));
}


template <typename AllocatableSize>
inline void* MemoryPool<AllocatableSize>::AllocPrimitive(size_t size) {
  size_t aligned = Align(size, kAlignment);
  void* empty = nullptr;
  bool malloced = false;
  
  if (aligned < kDefaultSize) {
    if (!current_chunk_->HasEnoughSize(aligned)) {
      current_chunk_->set_next(new Chunk<kDefaultSize>);
      current_chunk_ = current_chunk_->next();
    }
    empty = current_chunk_->GetBlock(aligned);
  } else {
    malloced = true;
    empty = malloc(aligned);
    Connect(empty);
  }

  return empty;
}


//Allocate chunk.
template <size_t AllocatableSize>
inline void* MemoryPool<AllocatableSize>::AllocClassType(size_t size) {
  if (!current_chunk_->HasEnoughSize(size)) {
    current_chunk_->set_next(new Chunk<AllocatableSize>);
    current_chunk_ = current_chunk_->next();
  }
  Allocatable* block = nullptr;
  if (size < kDefaultSize) {
    if (size < kAllocatableSize) {
      size = kAllocatableSize;
    }
    block = reinterpret_cast<Allocatable>(current_chunk_->GetBlock(size));
  } else {
    block = reinterpret_cast<Allocatable>(malloc(size));
    Connect(block);
  }
  
  return static_cast<void*>(block);
}


template <size_t AllocatableSize>
template <typename T>
void MemoryPool<AllocatableSize>::Append(T* allocated_list)  {
  auto allocated_list = new SinglyLinkedList<T>();
  if (allocatable_head_ == nullptr) {
    allocatable_head__ = current_allocatable_ = allocated_list;
  } else {
    ASSERT(true, current_allocatable_ != nullptr);
    current_allocatable_->set_next(allocated_list);
    current_ = allocated_list;
  }
}

} // namespace rasp



#endif
