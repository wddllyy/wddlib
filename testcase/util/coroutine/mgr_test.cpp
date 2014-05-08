#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <vector>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include<sys/time.h>
#include <memory.h>
#include "util/coroutine/comgr.h"


static const int MAX_CO_COUNT = 100;
SCoMgr mgr;

static int g_res = 0;
std::vector<int> ids;
void set_res(int i)
{
    g_res = i;
}
int rpc_get_res_timeout(int &res, int timeout)
{
    int iret = mgr.Yield(time(NULL)+timeout);
    if (iret < 0)
    {
        return iret;
    }

    res = g_res;
    return iret;
}
int rpc_get_res(int &res)
{
    int iret = mgr.Yield(time(NULL)+2);
    if (iret < 0)
    {
        return iret;
    }
    
    res = g_res;
    return iret;
}
static int assertvalue = 0;
static const int loopcount = 5;
int process1(int iCoPoolID)
{
    int result = 0;
    char buf[1024*50]={0};
    memset(buf, 0 , sizeof(buf));
    
    //int idx = *(int *)mgr.GetArg(iCoPoolID);
    for (int i = 0; i < loopcount; ++i)
    {
        //++co.temp;
        int res = 0;
        int ret = rpc_get_res(res);
        if (ret < 0)
        {
            printf("\n\n\n\n error %d:  \n\n\n\n",ret);
            return ret;
        }
        
        result += res;
    }
    //if(idx == 39)
    //{
    //    printf("%d res : %d \n",idx, result);
    //}
    assertvalue = result;
    return 0;
}


int processtime(int iCoPoolID)
{
    int result = 0;
    int idx = *(int *)mgr.GetArg(iCoPoolID);
    for (int i = 0; i < 5; ++i)
    {
        //++co.temp;
        int res = 0;
        int iret = rpc_get_res_timeout(res, (idx%2)+1);
        if (iret < 0)
        {
            printf("=============   error %d:%d %d =============\n",iCoPoolID, iret, idx);
            return iret;
        }

        result += res;

        //printf("%d %d %d -> res %d  result:%d\n",i, idx, iCoPoolID, res, result);
        printf("%d ",idx);
    }
    printf("%d res : %d \n",idx, result);
    return 0;
}
int process(int iCoPoolID)
{
    int result = 0;
    int idx = *(int *)mgr.GetArg(iCoPoolID);
    for (int i = 0; i < 100; ++i)
    {
        //++co.temp;
        int res = 0;
        int iret = rpc_get_res(res);
        if (iret < 0)
        {
            printf("=============   error %d:%d  =============\n",iCoPoolID, iret);
            return iret;
        }
        
        result += res;
        
        //printf("%d %d %d -> res %d  result:%d\n",i, idx, iCoPoolID, res, result);
        printf("%d ",idx);
    }
    printf("%d res : %d \n",idx, result);
    return 0;
}

int timeout(int iPoolID)
{
    int *pInt = (int*)mgr.GetArg(iPoolID);
    printf("timeout, idx:%d poolid:%d\n", *pInt,iPoolID);
    return 0;
}

int run(int iPoolID)
{
    
    int *pInt = (int*)mgr.GetArg(iPoolID);
    if (pInt != NULL)
    {
        set_res(*pInt);
    }
    mgr.Resume(iPoolID);
    return 0;
}

int runwithtimeout(int iPoolID)
{

    int *pInt = (int*)mgr.GetArg(iPoolID);
    if (pInt != NULL)
    {
        set_res(*pInt);
    }
    
    mgr.Resume(iPoolID);
    return 0;
}

void testtimeout()
{
    int iTimeoutCaseCount = 10;
    for (int i = 0 ; i < iTimeoutCaseCount ; ++i)
    {

        int id = mgr.Alloc(processtime, &i , sizeof(int));
        printf("alloc, idx:%d id:%d\n", i, id);
    }

    while (1)
    {
        int count = mgr.GeCoCount();
        if (count == 0)
        {
            break;
        }
        printf("---\n");
        mgr.ProcessAllActiveFun(runwithtimeout);
        sleep(1);
        mgr.ProcessAllExpire(time(NULL));
    }

    
}
void testfunc()
{
    for (int i = 0 ; i < MAX_CO_COUNT ; ++i)
    {
        int id = mgr.Alloc(process, &i , sizeof(int));
        //printf("id: %d\n",id);
        ids.push_back(id);
    }

    for (int i = 0 ; i < MAX_CO_COUNT ; ++i)
    {
        mgr.Resume(ids[i]);
    }

    printf("------------------------------------------------------------\n");

    unsigned int t = (unsigned int)time(NULL);
    (void)t;
    srand(0);

    while (1 )
    {
        if (ids.size() == 0)
        {
            break;
        }

        int idx = rand() % ids.size();

        //printf("id: %d/size :%d\n",idx, (int)ids.size());

        int *pInt = (int*)mgr.GetArg(ids[idx]);
        if (pInt != NULL)
        {
            set_res(*pInt);
        }


        int iRet = mgr.Resume(ids[idx]);
        if (iRet == 0)
        {
            std::vector<int>::iterator ite = ids.begin();
            std::advance(ite, idx);
            ids.erase(ite);
        }

    }
    ids.clear();
}
void testprocessallactive()
{
    int iTimeoutCaseCount = 10;
    for (int i = 0 ; i < iTimeoutCaseCount ; ++i)
    {

        int id = mgr.Alloc(process, &i , sizeof(int));
        printf("alloc, idx:%d id:%d\n", i, id);
    }

    while (1)
    {
        int count = mgr.GeCoCount();
        if (count == 0)
        {
            break;
        }

        mgr.ProcessAllActiveFun(run);
    }
}
void testperformance()
{
    printf("testperformance \n");
    for (int i = 0 ; i < MAX_CO_COUNT ; ++i)
    {
        int id = mgr.Alloc(process1, &i , sizeof(int));
        //printf("id: %d ",id);
        ids.push_back(id);
    }

    for (int i = 0 ; i < MAX_CO_COUNT ; ++i)
    {
        mgr.Resume(ids[i]);
    }

    time_t t = (unsigned int)time(NULL);
    srand(t);


    struct timeval starttime ;
    gettimeofday(&starttime, NULL);


    while (1 )
    {
        if ((int)ids.size() != MAX_CO_COUNT)
        {
            printf("ids.size() != MAX_CO_COUNT");
            assert((int)ids.size() == MAX_CO_COUNT);
        }

        int idx = rand() % ids.size();

        //printf("id: %d/size :%d\n",idx, (int)ids.size());

        int *pInt = (int*)mgr.GetArg(ids[idx]);
        if (pInt != NULL)
        {
            set_res(*pInt);
        }


        int iRet = mgr.Resume(ids[idx]);
        if (iRet == 0)
        {
            if (assertvalue != idx*loopcount)
            {
                printf("assertvalue != idx*loopcount");
                assert(assertvalue == idx*loopcount);
            }
            ids[idx] = mgr.Alloc(process1, &idx , sizeof(int));


        }
        static uint64_t ii =0;
        if (ii%100000 == 0)
        {
            struct timeval now ;
            gettimeofday(&now, NULL);

            float diff =   (now.tv_sec-starttime.tv_sec) + ((now.tv_usec - starttime.tv_usec))/1000000.f;
            printf("count %lu, tps %f, time:%f\n",ii, (float)ii / diff, diff);
            starttime = now;


            printf("%lu\n", ii);
            ii = 0;
        }
        ++ii;

    }
}



int main(int argc, const char *argv[])
{

    if (argc < 2)
    {
        fprintf (stderr, "Usage: %s [mode]\n mode: 0 -> FIXED_STACKSIZE_POOL; 1 -> FIXED_STACKSIZE_MALLOC; 2 -> DYNAMIC_STACKSIZE_MALLOC ; \n", argv[0]);
        exit (EXIT_FAILURE);
    }

    if (argv[1][0] == '0')
    {
        mgr.InitCoMgr(sizeof(int), 128*1024, MAX_CO_COUNT, FIXED_STACKSIZE_POOL, NULL, NULL);
    }
    else if (argv[1][0] == '2')
    {
        mgr.InitCoMgr(sizeof(int), 0, MAX_CO_COUNT, DYNAMIC_STACKSIZE_MALLOC, malloc, free);
    }
    else if (argv[1][0] == '1')
    {
        mgr.InitCoMgr(sizeof(int), 128*1024, MAX_CO_COUNT, FIXED_STACKSIZE_MALLOC, malloc, free);
    }
    else
    {
        fprintf (stderr, "Usage: %s [mode]\n mode: 0 -> FIXED_STACKSIZE_POOL; 1 -> FIXED_STACKSIZE_MALLOC; 2 -> DYNAMIC_STACKSIZE_MALLOC ; \n", argv[0]);
        exit (EXIT_FAILURE);
    }
    

    if (argv[1][1] == '0')
    {
        testfunc();
    }
    else if (argv[1][1] == '1')
    {
        testprocessallactive();
    }
    else if (argv[1][1] == '2')
    {
        testtimeout();
    }

    //
    //
    //
    //printf("\n\n\n\n");
    ////case 2
    //
    //

    //printf("\n\n\n\n");
    //// case 3

    //
    //printf("\n\n\n\n");
    
    testperformance();
    
    mgr.Destory();
    return 0;
}

