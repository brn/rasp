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
#include <random>
#include "../../src/utils/memorypool.h"

class Test1 {
 public:
  Test1(bool* ok):ok(ok){}
  ~Test1() {(*ok) = true;}
 private:
  bool* ok;
};


class LargeObject {
 public:
  LargeObject(bool* ok):ok(ok){}
  ~LargeObject() {(*ok) = true;}
 private:
  bool* ok;
  uint64_t padding1;
  uint64_t padding2;
  uint64_t padding3;
  uint64_t padding4;
  uint64_t padding6;
  uint64_t padding7;
  uint64_t padding8;
};


class Deletable {
 public:
  Deletable(bool* ok):ok(ok){}
  ~Deletable() = default;
  void Destruct() {(*ok) = true;}
 private:
  bool* ok;
};
/*
TEST(MemoryPoolTest, MemoryPoolTest_allocate_from_chunk) {
  bool ok;
  rasp::MemoryPool p(1024);
  Test1* t = p.Allocate<Test1>(&ok);
  p.Destroy();
  ASSERT_EQ(p.deleted_malloced_list.size(), 0u);
  ASSERT_EQ(p.deleted_chunk_list.size(), 1u);
  ASSERT_TRUE(ok);
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_from_chunk) {
  static const int kSize = 10000000;
  bool *ok_list = new bool[kSize];
  rasp::MemoryPool p(1024);
  for (int i = 0; i < kSize; i++) {
    p.Allocate<Test1>(&(ok_list[i]));
  }
  p.Destroy();
  ASSERT_EQ(p.deleted_malloced_list.size(), 0u);
  for (int i = 0, len = kSize; i < len; i++) {
    ASSERT_TRUE(ok_list[i]);
  }
  delete[] ok_list;
}
*/

TEST(MemoryPoolTest, MemoryPoolTest_allocate_random_many_from_chunk) {
  std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<size_t> size(1, 1000);
  static const int kSize = 10000000;
  //bool *ok_list = new bool[kSize];
  rasp::MemoryPool p(1024);
  for (int i = 0; i < kSize; i++) {
    char* ch = p.AllocateArray<char>(500);
    strcpy(ch, "a");
    ASSERT_STREQ("a", ch);
  }
  printf("OK!\n");
  p.Destroy();
  ASSERT_EQ(p.deleted_malloced_list.size(), 0u);
  /*for (int i = 0, len = kSize; i < len; i++) {
    ASSERT_TRUE(ok_list[i]);
    }*/
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_from_chunk_and_dealloc) {
  static const int kSize = 1000000;
  bool *ok_list = new bool[kSize];
  rasp::MemoryPool p(1024);
  for (int i = 0; i < kSize; i++) {
    Test1* t = p.Allocate<Test1>(&(ok_list[i]));
    p.Dealloc(t);
  }
  p.Destroy();
  ASSERT_EQ(p.deleted_malloced_list.size(), 0u);
  ASSERT_EQ(p.deleted_chunk_list.size(), 1u);
  for (int i = 0, len = kSize; i < len; i++) {
    ASSERT_TRUE(ok_list[i]);
  }
  delete[] ok_list;
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_big_object) {
  bool ok;
  rasp::MemoryPool p(8);
  LargeObject* t = p.Allocate<LargeObject>(&ok);
  intptr_t expected_addr = reinterpret_cast<intptr_t>(t);
  p.Destroy();
  ASSERT_EQ(p.deleted_malloced_list.size(), 1u);
  ASSERT_EQ(p.deleted_chunk_list.size(), 1u);
  ASSERT_EQ(expected_addr, p.deleted_malloced_list.at(0));
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_big_object) {
  static const int kSize = 10000000;
  bool *ok_list = new bool[kSize];
  rasp::MemoryPool p(8);
  for (int i = 0; i < kSize; i++) {
    p.Allocate<LargeObject>(&(ok_list[i]));
  }
  p.Destroy();
  ASSERT_EQ(p.deleted_chunk_list.size(), 1u);
  for (int i = 0, len = kSize; i < len; i++) {
    ASSERT_TRUE(ok_list[i]);
  }
  delete[] ok_list;
}
