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


//MemoryPool constructor.
MemoryPool::MemoryPool(size_t size)
    : dealloced_head_(nullptr),
      current_dealloced_(nullptr),
      size_(size) {
  ASSERT(true, size <= kMaxAllocatableSize);
  central_arena_ = central_arena_once_init_(&allocator_);
  deleted_.clear();
}


// Get heap from the pool.
// The heap structure is bellow
// |1-BIT SENTINEL-FLAG|1-BIT Allocatable FLAG|14-BIT SIZE BIT|FREE-MEMORY|
// This method return FREE-MEMORY area.
rasp::MemoryPool::MemoryBlock* MemoryPool::Chunk::GetBlock(size_t reserve) RASP_NOEXCEPT  {
  ASSERT(true, HasEnoughSize(reserve));
  
  Byte* ret = block_ + used_;
  
  if (tail_block_ != nullptr) {
    reinterpret_cast<MemoryBlock*>(tail_block_)->set_next_ptr(ret);
  }
  
  size_t reserved_size = RASP_ALIGN_OFFSET((sizeof(MemoryBlock) + reserve), kAlignment);
  size_t real_size = reserve + (reserved_size - (kValueOffset + reserve));
  used_ += reserved_size;
  tail_block_ = ret;
  
  MemoryBlock* memory_block = reinterpret_cast<MemoryBlock*>(ret);
  memory_block->set_size(real_size);
  memory_block->set_next_ptr(nullptr);
  return memory_block;
}


void MemoryPool::Chunk::Destruct() {
  if (0u == used_ || tail_block_ == nullptr) {
    return;
  }
  MemoryBlock* memory_block = reinterpret_cast<MemoryBlock*>(block_);
  while (1) {
    bool exit = IsTail(memory_block->ToBegin());
    if (!memory_block->IsMarkedAsDealloced()) {
      DestructMemoryBlock(memory_block);
    }
    if (exit) {
      break;
    }
    memory_block = memory_block->next_addr();
  }
}


MemoryPool& MemoryPool::operator = (MemoryPool&& memory_pool) {
  central_arena_ = memory_pool.central_arena_;
  dealloced_head_ = memory_pool.dealloced_head_;
  current_dealloced_ = memory_pool.current_dealloced_;
  size_ = memory_pool.size_;
  allocator_ = std::move(memory_pool.allocator_);
  memory_pool.deleted_.test_and_set();
  deleted_.clear();
  memory_pool.deleted_.test_and_set();
  memory_pool.central_arena_ = nullptr;
  memory_pool.dealloced_head_ = nullptr;
  memory_pool.current_dealloced_ = nullptr;
  return (*this);
}



MemoryPool::FreeChunkList* MemoryPool::LocalArena::InitHugeFreeChunkList(int index) {
  if (huge_free_chunk_map_.count(index) != 0) {
    return huge_free_chunk_map_[index];
  }
  FreeChunkList* new_free_chunk_list = new(mmap_.Commit(sizeof(ChunkList))) FreeChunkList();
  huge_free_chunk_map_[index] = new_free_chunk_list;
  return new_free_chunk_list;
}


bool MemoryPool::LocalArena::HasHugeFreeChunkList(int index) {
  return huge_free_chunk_map_.count(index) != 0;
}


void MemoryPool::CentralArena::Destroy() {
  LocalArena* arena = arena_head_;
  while (arena != nullptr) {
    auto chunk_list = arena->chunk_list();
    IterateChunkList(chunk_list);
    arena = arena->next();
  }
  tls_->~Slot();
}


void MemoryPool::CentralArena::IterateChunkList(MemoryPool::ChunkList* chunk_list) {
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


void MemoryPool::CentralArena::Dealloc(void* object) {
  ScopedSpinLock lock(dealloc_lock_);
  Byte* block = reinterpret_cast<Byte*>(object);
  block -= MemoryPool::kValueOffset;
  MemoryPool::MemoryBlock* memory_block = reinterpret_cast<MemoryPool::MemoryBlock*>(block);

  RASP_CHECK(true, !memory_block->IsMarkedAsDealloced());
  DestructMemoryBlock(memory_block);
  memory_block->MarkAsDealloced();
  ASSERT(true, memory_block->IsMarkedAsDealloced());
  int index = FindBestFitBlockIndex(memory_block->size());
  LocalArena* arena = TlsAlloc();
  arena->free_chunk_list(index)->AppendFreeList(memory_block);
}


const int MemoryPool::kValueOffset = sizeof(MemoryPool::MemoryBlock);
} //namespace rasp

