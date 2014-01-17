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

#include <gtest/gtest.h>
#include "../../src/utils/memorypool.h"

class Test1 : public rasp::Allocatable {
 public:
  Test1(bool* ok):ok(ok){}
  ~Test1() {(*ok) = true;}
 private:
  bool* ok;
};

TEST(MemoryPoolTest, MemoryPoolTest_allocate_from_chunk) {
  bool ok;
  rasp::MemoryPool<1024> p;
  Test1* t = new(&p) Test1(&ok);
  p.Destroy();
  ASSERT_EQ(p.deleted_allocatable_list.size(), 0u);
  ASSERT_EQ(p.deleted_non_class_ptr_list.size(), 0u);
  ASSERT_EQ(p.deleted_chunk_list.size(), 1u);
  ASSERT_TRUE(ok);
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_from_chunk) {
  static const int kSize = 200;
  bool ok_list[kSize] = {false};
  rasp::MemoryPool<1024> p;
  for (int i = 0; i < kSize; i++) {
    new(&p) Test1(&(ok_list[i]));
  }
  p.Destroy();
  ASSERT_EQ(p.deleted_allocatable_list.size(), 0u);
  ASSERT_EQ(p.deleted_non_class_ptr_list.size(), 0u);
  for (auto ok: ok_list) {
    ASSERT_TRUE(ok);
  }
}
