#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include "util/net/buffer.h"
#include "testcase_util.h"

int util_buffer_test(int argc, char *argv[])
{
    Buffer buf;
    
    //printf("%lu\n", buf.AllCapacity());
    TEST_ASSERT(buf.AllSize() == 1024);
    TEST_ASSERT(buf.ReadableBytes() == 0);
    TEST_ASSERT(buf.WritableBytes() == 1024);

    buf.Append("llyy", 5);
    //printf("%lu\n", buf.ReadableBytes());
    TEST_ASSERT(buf.ReadableBytes() == 5);
    TEST_ASSERT(buf.WritableBytes() == 1024-5);

    const char* pStr = buf.Peek();
    TEST_ASSERT(strcmp(pStr,"llyy") == 0);

    buf.Retrieve(2);
    TEST_ASSERT(buf.ReadableBytes() == 3);
    TEST_ASSERT(buf.WritableBytes() == 1024-5);

    pStr = buf.Peek();
    TEST_ASSERT(strcmp(pStr,"yy") == 0);

    buf.Retrieve(3);
    TEST_ASSERT(buf.ReadableBytes() == 0);
    TEST_ASSERT(buf.WritableBytes() == 1024);


    char a[300] = {0};
    for (int i = 0; i < 299; ++i)
    {
        a[i] = 'a';
    }
    
    buf.Append(a, 300);
    //printf("%lu\n", buf.ReadableBytes());
    TEST_ASSERT(buf.ReadableBytes() == 300);
    TEST_ASSERT(buf.WritableBytes() == 1024-300);
    pStr = buf.Peek();
    TEST_ASSERT(strcmp(pStr,a) == 0);

    buf.Retrieve(100);
    TEST_ASSERT(buf.ReadableBytes() == 200);
    TEST_ASSERT(buf.WritableBytes() == 1024-300);

    buf.Append(a, 300);
    TEST_ASSERT(buf.ReadableBytes() == 200+300);
    TEST_ASSERT(buf.WritableBytes() == 1024-300-300);

    char b[300] = {0};
    for (int i = 0; i < 199; ++i)
    {
        b[i] = 'a';
    }
    pStr = buf.Peek();
    TEST_ASSERT(strcmp(pStr,b) == 0);
    buf.Retrieve(200);
    TEST_ASSERT(buf.ReadableBytes() == 300);
    TEST_ASSERT(buf.WritableBytes() == 1024-300-300);

    buf.Append(a, 300);
    TEST_ASSERT(buf.ReadableBytes() == 300+300);
    TEST_ASSERT(buf.WritableBytes() == 1024-300-300-300);


    buf.Append(a, 300);
    TEST_ASSERT(buf.ReadableBytes() == 300+300+300);
    TEST_ASSERT(buf.WritableBytes() == 1024-300-300-300);

    buf.Append(a, 300);
    TEST_ASSERT(buf.ReadableBytes() == 300+300+300+300);
    TEST_ASSERT(buf.AllSize() == 300*4);
    TEST_ASSERT(buf.WritableBytes() == 0);

    buf.Retrieve(20000);
    TEST_ASSERT(buf.ReadableBytes() == 0);
    TEST_ASSERT(buf.AllSize() == 300*4);
    TEST_ASSERT(buf.WritableBytes() == 300*4);

    return 0;
}

int main(int argc, char* argv[])
{
    util_buffer_test(argc, argv);
    return 0;
}