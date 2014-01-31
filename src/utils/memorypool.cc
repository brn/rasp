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


void MemoryPool::Chunk::Destruct() {
  if (0u == used_ || tail_block_ == nullptr) {
    return;
  }
  MemoryBlock* memory_block = reinterpret_cast<MemoryBlock*>(block_);
  while (1) {
    bool exit = IsTail(memory_block->ToBegin());
    if (!memory_block->IsMarkedAsDealloced()) {
      memory_block->ToValue()->~Poolable();
    }
    if (exit) {
      break;
    }
    memory_block = memory_block->next_addr();
  }
}


MemoryPool& MemoryPool::operator = (MemoryPool&& memory_pool) {
  chunk_bundle_ = memory_pool.chunk_bundle_;
  dealloced_head_ = memory_pool.dealloced_head_;
  current_dealloced_ = memory_pool.current_dealloced_;
  size_ = memory_pool.size_;
  deleted_ = false;
  memory_pool.deleted_ = true;
  memory_pool.chunk_bundle_ = nullptr;
  memory_pool.dealloced_head_ = nullptr;
  memory_pool.current_dealloced_ = nullptr;
  return (*this);
}


void MemoryPool::Destroy() RASP_NOEXCEPT {
  if (!deleted_.load()) {
    deleted_.store(true);
    chunk_bundle_->Destroy();
  }
}


const std::array<int, 34> MemoryPool::ChunkBundle::kSizeMap = {{
    0, 1, 1, 1, 1, 1, 1, 1, 1,
    2, 2, 2, 2, 2, 2, 2, 2,
    3, 3, 3, 3, 3, 3, 3, 3,
    3, 3, 3, 3, 3, 3, 3, 3,
  }};


const std::array<int, 5> MemoryPool::ChunkBundle::kIndexSizeMap = {{
    0, 8, 16, 32
  }};

boost::thread_specific_ptr<MemoryPool::Arena> MemoryPool::ChunkBundle::tls_;

} //namespace rasp

