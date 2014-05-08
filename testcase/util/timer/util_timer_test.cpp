#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include "util/timer/lite_timer.h"



int util_timer_test(int argc, char *argv[])
{
    LiteTimerMgr<int> mgr;
    
    mgr.InitTimerMgr(100);
    mgr.StartTimer(1,1);
    mgr.StartTimer(2,2);
    mgr.StartTimer(3,3);
    mgr.StartTimer(4,4);
    mgr.StartTimer(5,5);
    mgr.StartTimer(5,6);
    mgr.StartTimer(5,7);
    mgr.StartTimer(5,8);
    mgr.StartTimer(5,9);
    mgr.StartTimer(4,10);
    mgr.StartTimer(3,11);
    mgr.StartTimer(2,12);
    mgr.StartTimer(1,13);
    
    
    for(int i = 0; i < 10; ++i)
    {
        int val = 0;
        int id = 0;
        do 
        {
            id = mgr.TickOneExpireTimer(i, val);
            if (id > 0)
            {
                printf("%d, %d--%d \n",i, id, val);
            }
            
        }
        while(id > 0);
    }
    mgr.Destory();
    printf("\n\n\n");


    int maxcount = 100*10000;
    mgr.InitTimerMgr(maxcount);
    for(int i = 0; i < maxcount; ++i)
    {
        mgr.StartTimer((i%30)+1, i+1);
        //printf("%d ",i);
    }
    printf("\n\n\n");
    printf("%d\n\n", mgr.GetActiveCount());
    int count = 0;
    for(int i = 0; i < maxcount; ++i)
    {
        int val = 0;
        int id = 0;
        do 
        {
            id = mgr.TickOneExpireTimer(i+1, val);
            if (id > 0)
            {
                //printf("%d-%d-%d ",i+1, id, val);
                ++count;
            }

        }
        while(id >= 0);

        if (mgr.GetActiveCount() <= 0)
        {
            break;
        }
        
    }

    printf("\n\n\n");
    printf("%d\n%d\n", mgr.GetActiveCount(), count);
    return 0;
}

int main(int argc, char* argv[])
{
    util_timer_test(argc, argv);
    return 0;
}