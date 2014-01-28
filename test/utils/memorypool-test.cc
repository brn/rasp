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

class Test0 {
 public:
  Test0(bool* ok):ok(ok){}
  ~Test0() {(*ok) = true;}
  bool check() {return *ok;}
  bool* ok;
};

class Test1 : public rasp::Poolable {
 public:
  Test1(bool* ok):rasp::Poolable(),ok(ok){}
  ~Test1() {(*ok) = true;}
  bool check() {return *ok;}
  bool* ok;
};


class Test2 : public rasp::Poolable  {
 public:
  Test2(bool* ok):rasp::Poolable(),ok(ok){}
  ~Test2() {(*ok) = true;}
  bool check() {return *ok;}
  bool* ok;
  uint64_t padding1;
  uint64_t padding2;
};


class Test3 : public rasp::Poolable  {
 public:
  Test3(bool* ok):rasp::Poolable(),ok(ok){}
  ~Test3() {(*ok) = true;}
  bool check() {return *ok;}
  bool* ok;
  uint64_t padding1;
  uint64_t padding2;
  uint64_t padding3;
  uint64_t padding4;
};


class LargeObject : public rasp::Poolable  {
 public:
  LargeObject(bool* ok):rasp::Poolable(),ok(ok){}
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


class Deletable : public rasp::Poolable  {
 public:
  Deletable(bool* ok):rasp::Poolable(),ok(ok){}
  ~Deletable() = default;
  void Destruct() {(*ok) = true;}
 private:
  bool* ok;
};


class Array : public rasp::Poolable {
 public:
  bool* ok;
  ~Array() {(*ok) = true;}
};

TEST(MemoryPoolTest, MemoryPoolTest_allocate_from_chunk) {
  bool ok;
  rasp::MemoryPool p(1024);
  Test1* t = new(&p) Test1(&ok);
  p.Destroy();
  ASSERT_TRUE(ok);
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_from_chunk) {
  static const int kSize = 10000000;
  bool *ok_list = new bool[kSize];
  rasp::MemoryPool p(1024);
  for (int i = 0; i < kSize; i++) {
    new(&p) Test1(&(ok_list[i]));
  }
  p.Destroy();
  for (int i = 0, len = kSize; i < len; i++) {
    ASSERT_TRUE(ok_list[i]);
  }
  delete[] ok_list;
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


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_from_chunk_and_dealloc2) {
  static const int kSize = 10000000;
  std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<size_t> size(1, 100);
  bool *ok_list = new bool[kSize];
  rasp::MemoryPool p(1024);
  void* last = nullptr;
  for (int i = 0; i < kSize; i++) {
    int s = size(mt);
    int t = s % 3 == 0;
    int f = s % 5 == 0;
    if (t) {
      last = new(&p) Test1(&(ok_list[i]));
    } else if (f) {
      last = new(&p) Test2(&(ok_list[i]));
    } else {
      if (last != nullptr) {
        p.Dealloc(last);
      }
      last = new(&p) Test3(&(ok_list[i]));
    }
  }
  p.Destroy();
  for (int i = 0, len = kSize; i < len; i++) {
    ASSERT_TRUE(ok_list[i]);
  }
  delete[] ok_list;
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_from_chunk_and_dealloc) {
  static const int kSize = 10000000;
  bool *ok_list = new bool[kSize];
  rasp::MemoryPool p(1024);
  for (int i = 0; i < kSize; i++) {
    Test1* t = new(&p) Test1(&(ok_list[i]));
    p.Dealloc(t);
  }
  p.Destroy();
  for (int i = 0, len = kSize; i < len; i++) {
    ASSERT_TRUE(ok_list[i]);
  }
  delete[] ok_list;
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_big_object) {
  bool ok;
  rasp::MemoryPool p(8);
  LargeObject* t = new(&p) LargeObject(&ok);
  intptr_t expected_addr = reinterpret_cast<intptr_t>(t);
  p.Destroy();
  ASSERT_TRUE(ok);
}


TEST(MemoryPoolTest, MemoryPoolTest_allocate_many_big_object) {
  static const int kSize = 10000000;
  bool *ok_list = new bool[kSize];
  rasp::MemoryPool p(8);
  for (int i = 0; i < kSize; i++) {
    new(&p) LargeObject(&(ok_list[i]));
  }
  p.Destroy();
  for (int i = 0, len = kSize; i < len; i++) {
    ASSERT_TRUE(ok_list[i]);
  }
  delete[] ok_list;
}


TEST(MemoryPoolTest, MemoryPoolTest_performance1) {
  static const int kSize = 10000000;
  rasp::MemoryPool p(1024);
  bool ok = true;
  bool last = false;
  for (int i = 0; i < kSize; i++) {
    Test1* t = new(&p) Test1(&ok);
    t->ok = &ok;
    last = t->check();
  }
  p.Destroy();
  ASSERT_TRUE(last);
}


TEST(MemoryPoolTest, MemoryPoolTest_performance2) {
  static const int kSize = 10000000;
  bool ok = true;
  bool last = false;
  for (int i = 0; i < kSize; i++) {
    std::shared_ptr<Test1> t = std::make_shared<Test1>(&ok);
    t->ok = &ok;
    last = t->check();
  }
  ASSERT_TRUE(last);
}


TEST(MemoryPoolTest, MemoryPoolTest_performance3) {
  static const int kSize = 10000000;
  bool ok = true;
  bool last = false;
  std::vector<Test0*> list;
  for (int i = 0; i < kSize; i++) {
    Test0* t = new(malloc(sizeof(Test0))) Test0(&ok);
    t->ok = &ok;
    last = t->check();
    t->~Test0();
    list.push_back(t);
  }
  for (auto c: list) {
    free(c);
  }
  ASSERT_TRUE(last);
}


TEST(MemoryPoolTest, MemoryPoolTest_performance4) {
  static const int kSize = 10000000;
  rasp::MemoryPool p(1024);
  bool ok = true;
  bool last = false;
  for (int i = 0; i < kSize; i++) {
    Test1* t = new(&p) Test1(&ok);
    t->ok = &ok;
    last = t->check();
    p.Dealloc(t);
  }
  ASSERT_TRUE(last);
}

