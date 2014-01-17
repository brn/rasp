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
#include <new>
#include <type_traits>


namespace rasp {


template <size_t kAllocatableSize>
inline void* Allocatable::operator new(size_t size, MemoryPool<kAllocatableSize>* pool) {
  return pool->AllocClassType(size);
}
inline void Allocatable::operator delete(void*) {}
template <size_t kAllocatableSize>
inline void Allocatable::operator delete(void* ptr, MemoryPool<kAllocatableSize>*){ operator delete(ptr); }



//MemoryPool constructor.
template <size_t kAllocatableSize>
inline MemoryPool<kAllocatableSize>::MemoryPool()
    : non_class_ptr_head_(nullptr),
      current_non_class_ptr_(nullptr),
      allocatable_head_(nullptr),
      current_allocatable_(nullptr),
      deleted_(false){
  current_chunk_ = chunk_head_ = new SinglyLinkedList<Chunk<kAllocatableSize>>(
      new Chunk<kAllocatableSize>());
}



template <size_t kAllocatableSize>
template <typename T, typename Deleter>
void MemoryPool<kAllocatableSize>::Delete(T head, Deleter d) {
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



template <size_t kAllocatableSize>
MemoryPool<kAllocatableSize>* MemoryPool<kAllocatableSize>::local_instance() {
  static thread_local MemoryPool<kAllocatableSize> local_instance;
  return &local_instance;
}


template <size_t kAllocatableSize>
template <typename T>
inline T* MemoryPool<kAllocatableSize>::Allocate(size_t size) {
  return reinterpret_cast<T*>(Alloc<T>(size, false));
}


//Allocate chunk.
template <size_t kAllocatableSize>
inline void* MemoryPool<kAllocatableSize>::AllocClassType(size_t size) {
  return Alloc<Allocatable>(size, true);
}


template <size_t kAllocatableSize>
template <typename T>
void* MemoryPool<kAllocatableSize>::Alloc(size_t size, bool is_allocatable) {
  size_t aligned = Align(size, kAlignment);
  T* block = nullptr;
  if (aligned <= kAllocatableSize) {
    if (!current_chunk_->value()->HasEnoughSize(aligned)) {      
      current_chunk_->set_next(new SinglyLinkedList<Chunk<kAllocatableSize>>(
          new Chunk<kAllocatableSize>()));
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


template <size_t kAllocatableSize>
template <typename T>
void MemoryPool<kAllocatableSize>::Append(T* pointer)  {
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
