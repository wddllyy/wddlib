#include <stdio.h>
#include "util/container/obj_pool.h"

TRAVEL_RET func(void* pvNode, void* pvArg)
{
    int *pValue = (int *)pvNode;
    printf("%d ",*pValue);
    if (*pValue % 2 == 0)
    {
        return REMOVE_ME;
    }
    return DO_THING;
}
int util_container_test(int argc, char* argv[])
{
    TObjPoolPtr ptr;
    objpool_init(&ptr, 100, sizeof(int));

    for (int i = 0 ; i < 110; ++i)
    {
        int id = objpool_alloc(ptr);
        printf("%d ", id);
    }
    for (int i = 0 ; i < 110; ++i)
    {
        int id = objpool_free(ptr, i+100);
        printf("%d ", id);
    }
    for (int i = 0 ; i < 50; ++i)
    {
        int id = objpool_alloc(ptr);
        printf("%d ", id);
    }
    for (int i = 0 ; i < 50; ++i)
    {
        int id = objpool_free(ptr, i+200);
        printf("%d ", id);
    }

    printf("%zu \n\n", objpool_get_used_items(ptr));
    objpool_destroy(&ptr);


    objpool_init(&ptr, 100, sizeof(int));
    for (int i = 0 ; i < 50; ++i)
    {
        int id = objpool_alloc(ptr);
        
        int id1 = objpool_free(ptr, i+100);
        printf("[%d %d]", id,id1);
    }
    printf("%zu \n\n", objpool_get_used_items(ptr));

    for (int i = 0 ; i < 50; ++i)
    {
        int id = objpool_alloc(ptr);
        int * pValue = (int * )objpool_get(ptr, id);
        *pValue = id;
    }

    objpool_travel_used(ptr, func, NULL);
    printf("\n\n%zu \n\n", objpool_get_used_items(ptr));
    objpool_travel_used(ptr, func, NULL);
    objpool_destroy(&ptr);
    return 0;
}

int main(int argc, char* argv[])
{
    util_container_test(argc, argv);
    return 0;
}