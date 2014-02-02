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


#ifndef UTILS_TLS_H_
#define UTILS_TLS_H_

#include <type_traits>
#include <thread>
#include <unordered_map>
#include "spinlock.h"
#include "utils.h"

namespace {

class TypeHolderBase {
 public:
  virtual ~TypeHolderBase(){}
  inline virtual void Delete(void*) = 0;
};


typedef size_t TlsKey;
typedef std::unordered_map<uint64_t, void*> InnerStorage;
typedef std::unordered_map<uint64_t, TypeHolderBase*> InnerDestructorStorage;
typedef std::unordered_map<TlsKey, InnerStorage> ThreadLocalPtr;
typedef std::unordered_map<TlsKey, InnerDestructorStorage> DestructorPtr;

static ThreadLocalPtr tls;
static DestructorPtr destructors;
static void Delete(void* p) {delete p;}
static uint64_t global_id = 0;
static rasp::SpinLock tls_lock;
static RASP_INLINE TlsKey GetTlsKey() RASP_NOEXCEPT {
  return std::hash<std::thread::id>()(std::this_thread::get_id());
}

inline static void Release() {
  rasp::ScopedSpinLock lock(tls_lock);
  size_t key = GetTlsKey();
  ThreadLocalPtr::iterator find = tls.find(key);
  DestructorPtr::iterator d_find = destructors.find(key);
  if (find != tls.end() && d_find != destructors.end()) {
    InnerStorage& inner = find->second;
    InnerDestructorStorage& d_inner = d_find->second;
    for (auto pair: d_inner) {
      InnerStorage::iterator it = inner.find(pair.first);
      if (it != inner.end()) {
        pair.second->Delete(static_cast<void*>(it->second));
      }
      delete pair.second;
    }
  }
  tls.erase(key);
  destructors.erase(key);
}
}

namespace rasp {

template <typename T>
class Tls {
  typedef typename std::remove_pointer<T>::type* ValueType;
  typedef std::function<void(ValueType)> ReleaseFn;

  
  class TypeHolder : public TypeHolderBase {
   public:
    TypeHolder(ReleaseFn d)
        : d_(d) {}
    inline void Delete(void* value) {
      d_(reinterpret_cast<ValueType>(value));
    }
   private:
    ReleaseFn d_;
  };
  
 public:

  template <typename Deleter>
  Tls(Deleter deleter = &Delete) {
    ScopedSpinLock lock(tls_lock);
    tls_id_ = global_id;
    global_id++;
    SetDestructor(ReleaseFn(deleter));
  }
    

  inline ValueType GetTlsData() {
    ScopedSpinLock lock(tls_lock);
    ThreadLocalPtr::iterator find = tls.find(GetTlsKey());
    std::cout << "1 " <<  GetTlsKey() << std::endl;
    if (find != tls.end()) {
      InnerStorage& inner = find->second;
      InnerStorage::iterator find = inner.find(tls_id_);
      return find != inner.end()? reinterpret_cast<ValueType>(find->second): nullptr;
    }
    return nullptr;
  }


  inline void SetTlsData(ValueType value) {
    ScopedSpinLock lock(tls_lock);
    size_t key = GetTlsKey();
    ThreadLocalPtr::iterator find = tls.find(key);
    if (find != tls.end()) {
      find->second[tls_id_] = static_cast<void*>(value);
    } else {
      InnerStorage inner;
      inner[tls_id_] = static_cast<void*>(value);
      tls[key] = std::move(inner);
      std::cout << "2 " <<  GetTlsKey() << " " << value << std::endl;
    }
  }


  inline void ClearTlsData() {
    ScopedSpinLock lock(tls_lock);
    ThreadLocalPtr::iterator find = tls.find(GetTlsKey());
    if (find != tls.end()) {
      find->second.clear();
    }
  }


  inline void EraseTlsData() {
    ScopedSpinLock lock(tls_lock);
    size_t key = GetTlsKey();
    ThreadLocalPtr::iterator find = tls.find(key);
    if (find != tls.end()) {
      InnerStorage& inner = find->second;
      inner.erase(tls_id_);
      if (inner.size() == 0) {
        tls.erase(key);
      }
    }
  }

  
 private:
  inline void SetDestructor(ReleaseFn fn) {
    size_t key = GetTlsKey();
    DestructorPtr::iterator find = destructors.find(key);
    if (find != destructors.end()) {
      find->second[tls_id_] = new TypeHolder(fn);
    } else {
      InnerDestructorStorage inner;
      inner[tls_id_] = new TypeHolder(fn);
      destructors[key] = std::move(inner);
    }
  }
  
  uint64_t tls_id_;
};
}

#ifdef _WIN32
#include <windows.h>
#include <winnt.h>
namespace {

void NTAPI tls_callback( void*, DWORD dwReason, void* ) { 
  if (dwReason == DLL_THREAD_DETACH) {
    Release();
  }
} 

// Add callback to the TLS callback list in TLS directory.
#pragma data_seg(push, old_seg)
#pragma data_seg(".CRT$XLB")
DWORD tls_callback_ptr = (DWORD)tls_callback;
#pragma data_seg(pop, old_seg)

extern "C" int _tls_used;
int dummy() {
  return _tls_used;
}

}
#endif
#endif
