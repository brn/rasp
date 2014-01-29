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

static const uint64_t kSize = 1000000u;


class Test0 {
 public:
  Test0(uint64_t* ok):ok(ok){}
  ~Test0() {(*ok)++;}
  uint64_t* ok;
};

class Test1 : public rasp::Poolable {
 public:
  Test1(uint64_t* ok):rasp::Poolable(),ok(ok){}
  ~Test1() {(*ok)++;}
  uint64_t* ok;
};


class Test2 : public rasp::Poolable  {
 public:
  Test2(uint64_t* ok):rasp::Poolable(),ok(ok){}
  ~Test2() {(*ok)++;}
  uint64_t* ok;
  uint64_t padding1;
  uint64_t padding2;
};


class Test3 : public rasp::Poolable  {
 public:
  Test3(uint64_t* ok):rasp::Poolable(),ok(ok){}
  ~Test3() {(*ok)++;}
  uint64_t* ok;
  uint64_t padding1;
  uint64_t padding2;
  uint64_t padding3;
  uint64_t padding4;
};


class LargeObject : public rasp::Poolable  {
 public:
  LargeObject(uint64_t* ok):rasp::Poolable(),ok(ok){}
  ~LargeObject() {(*ok)++;}
 private:
  uint64_t* ok;
  uint64_t padding1;
  uint64_t padding2;
  uint64_t padding3;
  uint64_t padding4;
  uint64_t padding6;
  uint64_t padding7;
  uint64_t padding8;
};


class Deletable : public rasp::Poolable  {
 public:
  Deletable(uint64_t* ok):rasp::Poolable(),ok(ok){}
  ~Deletable() = default;
  void Destruct() {(*ok)++;}
 private:
  uint64_t* ok;
};


class Array : public rasp::Poolable {
 public:
  bool* ok;
  ~Array() {(*ok) = true;}
};

TEST(MemoryPoolTest, MemoryPoolTest_allocate_from_chunk) {
  uint64_t ok = 0u;
  rasp::MemoryPool p(1024);
  new(&p) Test1(&ok);
  p.Destroy();
  ASSERT_EQ(ok, 1u);
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_from_chunk) {
  rasp::MemoryPool p(1024);
  uint64_t ok = 0u;
  for (uint64_t i = 0u; i < kSize; i++) {
    new(&p) Test1(&ok);
  }
  p.Destroy();
  ASSERT_EQ(kSize, ok);
}

/*
TEST(MemoryPoolTest, MemoryPoolTest_allocate_random_many_from_chunk) {
  std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<size_t> size(1, 100);
  static const int kSize = 10000000;
  std::vector<int> size_list;
  std::vector<bool*> ok_list;
  rasp::MemoryPool p(1024);
  for (int i = 0; i < kSize; i++) {
    int s = size(mt);
    Array* arr = p.AllocateArray<Array>(s);
    ok_list.push_back(new bool[s]);
    size_list.push_back(s);
    for (int j = 0; j < s; j++) {
      arr[j].ok = &(ok_list.at(i)[j]);
    }
  }

  p.Destroy();
  for (int i = 0, len = kSize; i < len; i++) {
    int size = size_list.at(i);
    for (int j = 0; j < size; j++) {
      ASSERT_TRUE(ok_list.at(i)[j]);
    }
    delete ok_list[i];
  }
  }*/


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_from_chunk_random) {
  uint64_t ok = 0u;
  std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<size_t> size(1, 100);
  rasp::MemoryPool p(1024);
  for (uint64_t i = 0u; i < kSize; i++) {
    int s = size(mt);
    int t = s % 3 == 0;
    int f = s % 5 == 0;
    if (t) {
      new(&p) Test1(&ok);
    } else if (f) {
      new(&p) Test2(&ok);
    } else {
      new(&p) Test3(&ok);
    }
  }
  p.Destroy();
  ASSERT_EQ(kSize, ok);
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_from_chunk_and_dealloc2) {
  uint64_t ok = 0u;
  std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<size_t> size(1, 100);
  rasp::MemoryPool p(1024);
  void* last = nullptr;
  for (uint64_t i = 0u; i < kSize; i++) {
    int s = size(mt);
    int ss = s % 6 == 0;
    int t = s % 3 == 0;
    int f = s % 5 == 0;

    if (ss) {
      if (last != nullptr) {
        p.Dealloc(last);
      }
    }
    
    if (t) {
      last = new(&p) Test1(&ok);
    } else if (f) {
      last = new(&p) Test2(&ok);
    } else {
      last = new(&p) Test3(&ok);
    }
  }
  p.Destroy();
  ASSERT_EQ(kSize, ok);
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_from_chunk_and_dealloc) {
  uint64_t ok = 0u;
  rasp::MemoryPool p(1024);
  for (uint64_t i = 0u; i < kSize; i++) {
    Test1* t = new(&p) Test1(&ok);
    p.Dealloc(t);
  }
  p.Destroy();
  ASSERT_EQ(kSize, ok);
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_big_object) {
  uint64_t ok = 0u;
  rasp::MemoryPool p(8);
  new(&p) LargeObject(&ok);
  p.Destroy();
  ASSERT_EQ(ok, 1u);
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_big_object) {
  uint64_t ok = 0u;
  rasp::MemoryPool p(8);
  for (uint64_t i = 0u; i < kSize; i++) {
    new(&p) LargeObject(&ok);
  }
  p.Destroy();
  ASSERT_EQ(kSize, ok);
}


TEST(MemoryPoolTest, MemoryPoolTest_performance1) {
  rasp::MemoryPool p(1024);
  uint64_t ok = 0u;
  for (uint64_t i = 0u; i < kSize; i++) {
    new(&p) Test1(&ok);
  }
  p.Destroy();
  ASSERT_EQ(kSize, ok);
}


TEST(MemoryPoolTest, MemoryPoolTest_performance2) {
  uint64_t ok = 0u;
  {
    std::vector<std::shared_ptr<Test0>> list(kSize);
    for (uint64_t i = 0u; i < kSize; i++) {
      list[i] = std::make_shared<Test0>(&ok);
    }
  }
  ASSERT_EQ(kSize, ok);
}


TEST(MemoryPoolTest, MemoryPoolTest_performance3) {
  uint64_t ok = 0u;
  std::vector<Test0*> list(kSize);
  for (uint64_t i = 0u; i < kSize; i++) {
    list[i] = new Test0(&ok);
  }
  for (auto c: list) {
    delete c;
  }
  ASSERT_EQ(kSize, ok);
}

