#ifndef UTIL_INLINE_H_
#define UTIL_INLINE_H_

#ifdef WIN32
#define INLINE inline
#elif defined(__GNUC__)
#define INLINE __attribute((always_inline, flatten));
#endif

#endif
