#ifndef UTILS_VISIBILITY_H_
#define UTILS_VISIBILITY_H_

#ifdef TEST
#define VISIBLE_FOR_TEST(type) public
#else
#define VISIBLE_FOR_TEST(type) type
#endif

#endif
