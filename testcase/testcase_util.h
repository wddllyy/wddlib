#ifndef __TESTCASE_UTIL_H
#define __TESTCASE_UTIL_H

#define TEST_ASSERT(condition) \
do { \
    if (!(condition)) { \
        fprintf(stderr, \
        "%s:%d: error: Test in  %s failed : %s\n", \
        __FILE__, \
        __LINE__, \
        __FUNCTION__ , \
        #condition); \
    } \
} while (0)

#endif