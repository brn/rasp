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

#include "memorypool.h"


namespace rasp {

// Create Chunk from byte block.
// Chunk and heap block is create from one big memory block.
// The structure is below
// |8-BIT VERIFY BIT|Chunk MEMORY BLOCK|The memory block managed BY Chunk|
MemoryPool::Chunk* MemoryPool::Chunk::New(size_t size, HeapAllocator* allocator) {
  ASSERT(true, size <= kMaxAllocatableSize);
  static const size_t kChunkSize = RASP_ALIGN(sizeof(Chunk), kAlignment);
  const size_t aligned_size = RASP_ALIGN(size, kAlignment);
  
  // All heap size we want.
  const size_t heap_size = RASP_ALIGN((kVerificationTagSize + kChunkSize + aligned_size), kAlignment);
  Byte* ptr = reinterpret_cast<Byte*>(allocator->Allocate(heap_size));
  
  if (ptr == NULL) {
    throw std::bad_alloc();
  }

  // Verification bit.
  VerificationTag* tag = reinterpret_cast<VerificationTag*>(ptr);
  (*tag) = kVerificationBit;
  void* chunk_area = PtrAdd(ptr, kVerificationTagSize);
  
  // Instantiate Chunk from the memory block.
  return new(chunk_area) Chunk(reinterpret_cast<Byte*>(PtrAdd(chunk_area, kChunkSize)), aligned_size);
}


void MemoryPool::Chunk::Destruct() {
  if (0u == used_ || tail_block_ == nullptr) {
    return;
  }
  MemoryBlock* memory_block = reinterpret_cast<MemoryBlock*>(block_);
  while (1) {
    bool exit = IsTail(memory_block->ToBegin());
    if (!memory_block->IsMarkedAsDealloced()) {
      memory_block->ToDisposable()->Dispose(memory_block->ToBegin(), memory_block->ToValue<void>());
    }
    if (exit) {
      break;
    }
    memory_block = memory_block->next_addr();
  }
}


MemoryPool& MemoryPool::operator = (MemoryPool&& memory_pool) {
  current_chunk_ = memory_pool.current_chunk_;
  chunk_head_ = memory_pool.chunk_head_;
  dealloced_head_ = memory_pool.dealloced_head_;
  current_dealloced_ = memory_pool.current_dealloced_;
  size_ = memory_pool.size_;
  deleted_ = false;
  memory_pool.deleted_ = true;
  memory_pool.current_chunk_ = nullptr;
  memory_pool.chunk_head_ = nullptr;
  memory_pool.dealloced_head_ = nullptr;
  memory_pool.current_dealloced_ = nullptr;
  return (*this);
}


void MemoryPool::Destroy() RASP_NOEXCEPT {
  if (!deleted_) {
    deleted_ = true;
    auto chunk = chunk_head_;
    if (chunk != nullptr) {
      while (chunk != nullptr) {
        ASSERT(true, chunk != nullptr);
#ifdef UNIT_TEST
        ReserveForTest(chunk);
#endif    
        auto tmp = chunk;
        chunk = chunk->next();
        Chunk::Delete(tmp, &allocator_);
      }
    }
    chunk_head_ = nullptr;
  }
}
}
