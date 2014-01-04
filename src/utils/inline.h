#ifndef UTIL_INLINE_H_
#define UTIL_INLINE_H_

#ifdef _WIN32
#define INLINE inline
#elif defined(__GNUC__)
#define INLINE inline __attribute__((always_inline));
#endif

#endif
