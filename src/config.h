#ifndef INCLUDE_CONFIG_H_
#define INCLUDE_CONFIG_H_

#define HAVE_BOOST_MUTEX

#define HAVE_STD_UNIQUE_PTR

#define HAVE_BOOST_CONDITION_VARIABLE

#define HAVE_BOOST_THREAD_TSS_HPP

#define HAVE_STD_MAKE_SHARED

#define HAVE_BOOST_REF

#define HAVE_UNORDERED_MAP

#define HAVE_STDINT_H

#define HAVE_VIRTUALALLOC

#define HAVE_BOOST_PREPROCESSOR_REPETITION_ENUM_PARAMS_HPP

#define HAVE_THREAD

#define HAVE_BOOST_DETAIL_ATOMIC_COUNT_HPP

#define HAVE_TYPE_TRAITS

#define HAVE_HEAPALLOC

#define HAVE_STD_CONDITION_VARIABLE

#define HAVE_STD_BIND

#define HAVE_BOOST_PREPROCESSOR_REPETITION_REPEAT_HPP

#define HAVE_TUPLE

#define HAVE_STD_FUNCTION

#define HAVE_BOOST_PREPROCESSOR_REPETITION_ENUM_BINARY_PARAMS_HPP

#define HAVE_STD_ALLOCATE_SHARED

#define HAVE_FORCE_INLINE

#define HAVE_BOOST_THREAD_SPECIFIC_PTR

#define HAVE_STD_SHARED_PTR

//#undef HAVE_STD_MUTEX

//#undef HAVE_INLINE_ATTRIUTE

//#undef HAVE_STD_REF

//#undef HAVE_NOEXCEPT


  #if defined(__x86_64__) || defined(_M_X64)
    #define PLATFORM_64BIT 1
  #elif defined(__i386) || defined(_M_IX86)
    #define PLATFORM_32BIT 1
  #endif
  

#endif
