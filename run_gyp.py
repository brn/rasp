#!/usr/bin/env python

import os
import sys
import configbuilder
script_dir = os.path.dirname(os.path.abspath(__file__))
root = os.path.normpath(script_dir);
build = os.path.join(os.path.normpath(script_dir), 'build');
sys.path.insert(0, os.path.join(os.path.join(build, 'gyp'), 'pylib'))

import glob
import shlex
import gyp


def CheckThreads(builder, args) :
  std_thread = builder.CheckHeader(False, ['thread', 'thread'], '')
  stds = {};
  boosts = {};
  if not std_thread[0] :
    result = builder.CheckLibAndHeader(True, [
    {
      'header' : ['boost/thread.hpp'],
      'lib' : ['boost_thread-vc120-mt-1_55.lib', 'boost_thread-vc120-mt-gd-1_55.lib', 'libboost_thread-mt.a']
    }
    ], 'boost/thread is required.')
    args.append('-Dboost_thread_lib_name=' + result[1])

  stds['mutex'] = builder.CheckStruct(False, [
      {
        'name' : 'std::mutex',
        'header' : ['thread.hpp', 'mutex'],
        'struct': 'std::mutex'
      }
      ], 'mutex required.')

  boosts['mutex'] = builder.CheckStruct(False, [
      {
        'name' : 'boost::mutex',
        'header' : ['boost/thread.hpp', 'boost/thread/condition.hpp'],
        'struct': 'boost::mutex'
        }
      ], 'mutex required.')

  stds['condition_variable'] = builder.CheckStruct(False, [
      {
        'name' : 'std::condition_variable',
        'header' : ['thread', 'condition_variable'],
        'struct': 'std::condition_variable'
      }
      ], 'condition_variable required.')

  boosts['condition_variable'] = builder.CheckStruct(False, [
      {
        'name' : 'boost::condition_variable',
        'header' : ['boost/thread/condition.hpp'],
        'struct': 'boost::condition_variable'
        }
      ], 'condition_variable required.')

  builder.CheckHeader(True, ['boost/thread/tss.hpp'], 'boost/thread/tss.hpp is required.')

  builder.CheckStruct(True, [
      {
        'name' : 'boost::thread_specific_ptr',
        'header' : ['boost/thread/tss.hpp'],
        'struct' : 'boost::thread_specific_ptr<int>'
        }
      ], 'boost::thread_specific_ptr is required.')
  
  if std_thread[0] :
    for std in stds :
      if std[0] == False :
        print 'std::' + std[1] + ' is required. Change a compiler or install boost library.'
        sys.exit(1)

  else :
    for boost in boosts :
      if boost[0] == False :
        print 'boost::' + boost[1] + ' is required. You need to install boost library1.55.0 or later.'
        sys.exit(1)

def BuildConfig(args) :
  builder = configbuilder.ConfigBuilder('src/config.h', always_build=True)
  builder.CheckHeader(False, ['stdint.h'], '')
  builder.CheckHeader(True, ["unordered_map", "unordered_map", "boost/unordered_map.hpp"], 'unordered_map required.')
  builder.CheckStruct(True, [
      {
        'name' : 'std::bind',
        'header' : ['functional'],
        'function' : 'std::bind(fopen, std::placeholders::_1, "rb")'
        }, 
      {
        'name' : 'boost::bind',
        'header' : ['boost/bind.hpp'],
        'function' : 'boost::bind(fopen, _1, "rb")'
        }], 'bind required.')
  builder.CheckHeader(True, ['type_traits', 'boost/type_traits.hpp'], 'type_traits required.')
  CheckThreads(builder, args)
  builder.CheckHeader(True, ['tuple', 'std/tuple', 'boost/tuple'], 'tuple required.')
  builder.CheckStruct(True, [
      {
        'name' : 'std::function',
        'header' : ['functional'],
        'struct': 'std::function<int (int)>'
      },
      {
        'name' : 'boost::function',
        'header' : ["boost/function.hpp"],
        'struct' : 'boost::function<int (int)>'
      }
    ], 'function required.')
  builder.CheckStruct(True, [
      {
        'name' : 'std::shared_ptr',
        'header' : ['memory'],
        'struct': 'std::shared_ptr<const char>'
      },
      {
        'name' : 'tr1::shared_ptr',
        'header' : ['memory'],
        'struct': 'std::tr1::shared_ptr<const char>'
      },
      {
        'name' : 'boost::shared_ptr',
        'header' : ["boost/shared_ptr.hpp"],
        'struct' : 'boost::shared_ptr<const char>'
      }
    ], 'shared_ptr required.')
  builder.CheckStruct(True, [
      {
        'name' : 'std::allocate_shared',
        'header' : ['memory'],
        'function': 'std::allocate_shared<char>(std::allocator<char>())'
      },
      {
        'name' : 'boost::allocate_shared',
        'header' : ["memory", "boost/shared_ptr.hpp", "boost/make_shared.hpp"],
        'function' : 'boost::allocate_shared<char>(std::allocator<char>())'
      }
    ], 'allocate_shared required.')

  builder.CheckStruct(True, [
      {
        'name' : 'std::make_shared',
        'header' : ['memory'],
        'function': 'std::make_shared<char>(5)'
      },
      {
        'name' : 'boost::make_shared',
        'header' : ["memory", "boost/shared_ptr.hpp", "boost/make_shared.hpp"],
        'function' : 'boost::make_shared<char>(5)'
      }
    ], 'allocate_shared required.')

  builder.CheckStruct(True, [
      {
        'name' : 'std::unique_ptr',
        'header' : ['memory'],
        'struct': 'std::unique_ptr<int>'
      },
      {
        'name' : 'boost::unique_ptr',
        'header' : ["boost/interprocess/smart_ptr/unique_ptr.hpp", 'boost/checked_delete.hpp'],
        'struct' : 'boost::interprocess::unique_ptr<int, boost::checked_deleter<int> >'
      }
    ], 'unique_ptr is required.')

  builder.CheckStruct(True, [
      {
        'name' : 'std::ref',
        'header' : ['functional'],
        'function': 'std::ref(printf)'
      },
      {
        'name' : 'boost::ref',
        'header' : ["boost/ref.hpp"],
        'function' : 'boost::ref(printf)'
      }
    ], 'ref is required.')

  builder.CheckHeader(True, ["boost/preprocessor/repetition/repeat.hpp"], 'boost/preprocessor/repetition/repeat.hpp required.')
  builder.CheckHeader(True, ["boost/preprocessor/repetition/enum_params.hpp"], 'boost/preprocessor/repetition/enum_params.hpp required.')
  builder.CheckHeader(True, ["boost/preprocessor/repetition/enum_binary_params.hpp"], 'boost/preprocessor/repetition/enum_binary_params.hpp required.')
  builder.CheckHeader(True, ["boost/detail/atomic_count.hpp"], 'boost/preprocessor/repetition/repeat.hpp required.')
  builder.CheckStruct(False, [
    {
      'name': 'inline_attriute',
      'code' : '''
        inline __attribute__((always_inline)) int Test() {return 0;}
      '''
    },
    {
      'name': 'force_inline',
      'code' : '''
        inline __forceinline int Test() {return 0;}
      '''
    }
  ], 'inline attribute is required.')
  builder.CheckStruct(False, [
    {
      'name': 'HeapAlloc',
      'header' : ['Windows.h'],
      'function': 'HeapAlloc'
    },
    {
      'name': 'mmap',
      'header' : ['sys/mman.h'],
      'function': 'mmap'
    }
  ], 'HeapAlloc is required.')
  builder.CheckStruct(False, [
    {
      'name': 'VirtualAlloc',
      'header' : ['Windows.h'],
      'function': 'VirtualAlloc'
    }
  ], 'VirtualAlloc is required.')
  builder.CheckStruct(False, [
    {
      'name': 'munmap',
      'header' : ['sys/mman.h'],
      'function': 'munmap'
    }
  ], 'munmap is required.')
  builder.CheckStruct(False, [
    {
      'name': 'noexcept',
      'code' : '''
        int Test() noexcept {return 0;}
      '''
    }
  ], 'noexcept is required.')
  builder.CheckStruct(False, [
    {
      'name': 'VM_MAKE_TAG',
      'header' : ['mach/vm_statistics.h'],
      'code': '''
        void test(){VM_MAKE_TAG(64);}
      '''
    }
  ], 'VM_MAKE_TAG is required.')
  builder.AddMacroCode("""
  #if defined(__x86_64__) || defined(_M_X64)
    #define PLATFORM_64BIT 1
  #elif defined(__i386) || defined(_M_IX86)
    #define PLATFORM_32BIT 1
  #endif
  """)
  builder.Build()


# Directory within which we want all generated files (including Makefiles)
# to be written.

def run_gyp(args):
  rc = gyp.main(args)
  if rc != 0:
    print 'Error running GYP'
    sys.exit(rc)

if __name__ == '__main__':
  args = sys.argv[1:]

  # GYP bug.
  # On msvs it will crash if it gets an absolute path.
  # On Mac/make it will crash if it doesn't get an absolute path.
  if sys.platform == 'win32':
    args.append(os.path.join(root, 'rasp.gyp'))
    common_fn  = os.path.join(root, 'commons.gypi')
  else:
    args.append(os.path.join(os.path.abspath(root), 'rasp.gyp'))
    common_fn  = os.path.join(os.path.abspath(root), 'commons.gypi')
  print common_fn
  if os.path.exists(common_fn):
    args.extend(['-I', common_fn])

  args.append('--depth=./')
  
  additional_include = os.getenv("INCLUDE")
  additional_lib = os.getenv("LIB")
  if  additional_include :
    args.append('-Dadditional_include=' + additional_include)
  else :
    args.append('-Dadditional_include=""')
  
  if additional_lib :
    args.append('-Dadditional_lib=' + additional_lib)
  else :
    args.append('-Dadditional_lib=""')
    
  # There's a bug with windows which doesn't allow this feature.
  if sys.platform != 'win32':
    # Tell gyp to write the Makefiles into output_dir
    args.extend(['--generator-output', build])
    # Tell make to write its output into the same dir
    args.extend(['-Goutput_dir=' + build])
    # Create Makefiles, not XCode projects
  if sys.platform != 'darwin' and sys.platform != 'win32':
    args.extend('-f make'.split())
  elif sys.platform == 'darwin':
    args.extend('-f xcode'.split())

  args.append('-Dtarget_arch=ia32')
  args.append('-Dcomponent=static_library')
  args.append('-Dlibrary=static_library')  
  args.append('-Dcurrent_dir=' + os.getcwd().replace('\\', '/'))
  BuildConfig(args)
  gyp_args = list(args)
  print gyp_args
  run_gyp(gyp_args)
