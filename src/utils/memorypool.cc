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
  if (0u == used_) {
    return;
  }
  void* block = reinterpret_cast<void*>(block_);
  while (1) {
    void* block_begin = block;
    TagBit* bit_ptr = reinterpret_cast<TagBit*>(block);
    TagBit bit = (*bit_ptr);
    bool exit = (bit & kSentinelBit) == kSentinelBit;
    uint16_t size = bit & kFlagMask;
    block = PtrAdd(block, kTagBitSize);
    if ((bit & kDeallocedBit) != kDeallocedBit) {
      DisposableBase* base = reinterpret_cast<DisposableBase*>(block);
      block = PtrAdd(block, kDisposableBaseSize);
      base->Dispose(block_begin, block);
    } else {
      block = PtrAdd(block, kDisposableBaseSize);
    }
    if (exit) {
      break;
    }
    block = PtrAdd(block, size);
  }
}


void MemoryPool::Destroy() RASP_NOEXCEPT {
  if (!deleted_) {
    deleted_ = true;
    Delete(malloced_head_, [this](void* ptr) {
        void *block = ptr;
        block = PtrAdd(block, kTagBitSize);
        DisposableBase* base = reinterpret_cast<DisposableBase*>(block);
        block = PtrAdd(block, kDisposableBaseSize);
#ifdef UNIT_TEST
        ReserveForTest(block);
#endif
        base->Dispose(ptr, block);
      });
    Delete(chunk_head_, [this](Chunk* ptr) {
#ifdef UNIT_TEST
        ReserveForTest(ptr);
#endif    
        Chunk::Delete(ptr);
      });
  }
}
}
