#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <memory.h>
#include "util/coroutine/coroutine.h"
#include<sys/time.h>

struct MyCoroutine
{
    coroutine m_co;
    char buff[1024 * 128];
    int index;
    int temp;
    MyCoroutine()
    {
        temp = -1;
    }
};
static schedule g_Sch;

static const int MAX_CO_COUNT = 10000;
static MyCoroutine g_CoArray[MAX_CO_COUNT];
static int sum = 0;
char sendbuf[1024*1024];
char sendbuf1[1024*1024];
struct timeval tt; 

int GetStackSize(char * top)
{
    char dump;
    return (int)(top - &dump);
}


static int res = 0;
void set_res(int i)
{
    res = i;
}

int get_res()
{
    return res;
}


int process(void* arg)
{
    int result = 0;
    int idx = *(int *)arg;
    MyCoroutine& co = g_CoArray[idx];
    for (int i = 0; i < 100; ++i)
    {
        //++co.temp;
        co_yield(&g_Sch);
        result += get_res();
    }
    printf("%d res : %d \n", co.index, result);
    return 0;
}
void pcase()
{
    snprintf(sendbuf, sizeof(sendbuf), "I am a client, want to send data to server!"); 
    //snprintf(sendbuf, sizeof(sendbuf), "I am a client, want to send data to server!"); 
    //snprintf(sendbuf, sizeof(sendbuf), "I am a client, want to send data to server!"); 
    //snprintf(sendbuf, sizeof(sendbuf), "I am a client, want to send data to server! %d", sum); 
    //memcpy(sendbuf, sendbuf1, 2000);
    //memset(sendbuf, 0, 2000);
    //gettimeofday(&tt, NULL);
}

int ptest(void* arg)
{
    while(1)
    {
        ++sum;
        //snprintf(sendbuf, sizeof(sendbuf), "I am a client, want to send data to server!"); 
        pcase();
        co_yield(&g_Sch);
    }
    return 0;
}

void testperformance(bool isdynamic)
{    
    if (isdynamic)
    {
        g_Sch.allocfunc = malloc;
        g_Sch.freefunc = free;
        g_Sch.isDynamicStack = true;

        g_Sch.firststack = (char*)malloc(1024*128);
        g_Sch.firststackmaxsize = 1024*128;
        g_Sch.secondstack = (char*)malloc(1024*128);
        g_Sch.secondstackmaxsize = 1024*128;
    }
    printf("testperformance \n\n");



    for (int i = 0 ; i < MAX_CO_COUNT ; ++i)
    {
        g_CoArray[i].index = i;
        g_CoArray[i].temp = i;
        co_init(&g_CoArray[i].m_co, ptest, &g_CoArray[i].index, g_CoArray[i].buff, sizeof(g_CoArray[i].buff), &g_Sch);
    }
    for (int i = 0 ; i < MAX_CO_COUNT ; ++i)
    {
        //MyCoroutine& co = g_CoArray[i];
        co_resume(&g_CoArray[i].m_co, &g_Sch);
    }

    struct timeval starttime ;
    gettimeofday(&starttime, NULL);

    unsigned long test = 0;
    while (1 )
    {
        unsigned int idx = (test)%2;//(test)%MAX_CO_COUNT;
        coroutine& co = g_CoArray[idx].m_co;
        if(co.status != CO_STATE_END)
        {
            co_resume(&co, &g_Sch);
        }

        if(++test == 1000000)
        {
            struct timeval now ;
            gettimeofday(&now, NULL);

            float diff =   (now.tv_sec-starttime.tv_sec) + ((now.tv_usec - starttime.tv_usec))/1000000.f;
            printf("count %lu, tps %f, time:%f\n",test, (float)test / diff, diff);
            starttime = now;

            printf("testperformance sum:%d \n",sum);
            test = 0;
        }


        
    }
};

void func(MyCoroutine& co)
{
    sum += co.temp;
}

void testperformance1(bool isdynamic)
{    
    if (isdynamic)
    {
        g_Sch.allocfunc = malloc;
        g_Sch.freefunc = free;
        g_Sch.isDynamicStack = true;

        g_Sch.firststack = (char*)malloc(1024*128);
        g_Sch.firststackmaxsize = 1024*128;
        g_Sch.secondstack = (char*)malloc(1024*128);
        g_Sch.secondstackmaxsize = 1024*128;
    }
    
    for (int i = 0 ; i < MAX_CO_COUNT ; ++i)
    {
        MyCoroutine& co = g_CoArray[i];
        co.temp = 1;
        
    }
    struct timeval starttime ;
    gettimeofday(&starttime, NULL);

    unsigned long test = 0;
    while (1)
    {

        unsigned int idx = (test)%2;

        MyCoroutine& co = g_CoArray[idx];
        func(co);
        //++sum;
        //snprintf(sendbuf, sizeof(sendbuf), "I am a client, want to send data to server!"); 
        pcase();
        if(++test == 1000000)
        {
            struct timeval now ;
            gettimeofday(&now, NULL);

            float diff =   (now.tv_sec-starttime.tv_sec) + ((now.tv_usec - starttime.tv_usec))/1000000.f;
            printf("count %lu, tps %f, time:%f\n",test, (float)test / diff, diff);
            starttime = now;

            printf("testperformance1 sum:%d \n",sum);
            test = 0;
        }

    }
}

const int ParentCount = 100;
const int SonCount = 9;
const int SonSonCount = 10;

int processtask(int &result,int idx)
{
    for (int i = 0; i < 5; ++i)
    {
        //printf("processtask: %d  res:%d\n",idx,result);
        co_yield(&g_Sch);
        result += get_res();
    }
    return 0;
}
int processsonson(void* arg)
{
    int result = 0;
    int idx = *(int *)arg;
    MyCoroutine& co = g_CoArray[idx];
    printf("%d sonson start \n", co.index);
    //processtask(result, idx);
    //processtask(result, idx);
    printf("%d sonson res : %d \n", co.index, result);
    return 0;
}

int processson(void* arg)
{
    int result = 0;
    int idx = *(int *)arg;
    MyCoroutine& co = g_CoArray[idx];

    processtask(result, idx);
    printf("%d son start \n", co.index);
    for (int i = idx+1; i < idx+1+10; i += 1)
    {
        g_CoArray[i].index = i;
        g_CoArray[i].temp = i;
        co_init(&g_CoArray[i].m_co, processsonson, &g_CoArray[i].index, g_CoArray[i].buff, sizeof(g_CoArray[i].buff), &g_Sch);
        //co_init(&g_CoArray[i].m_co, processsonson, &g_CoArray[i].index, (char*)g_Sch.ctx.uc_stack.ss_sp, g_Sch.ctx.uc_stack.ss_size, &g_Sch);
        co_resume(&g_CoArray[i].m_co, &g_Sch);
    }
    
    

    processtask(result, idx);
    printf("%d son res : %d \n", co.index, result);
    return 0;
}

int processparent(void* arg)
{

    int result = 0;

    int idx = *(int *)arg;
    MyCoroutine& co = g_CoArray[idx];

    printf("%d parent start \n", co.index);
   

    //printf("%d fork son start \n", co.index);
    for (int i = idx+1; i < idx+100; i += 11)
    {
        g_CoArray[i].index = i;
        g_CoArray[i].temp = i;
        co_init(&g_CoArray[i].m_co, processson, &g_CoArray[i].index, g_CoArray[i].buff, sizeof(g_CoArray[i].buff), &g_Sch);
        //co_init(&g_CoArray[i].m_co, processson, &g_CoArray[i].index, (char*)g_Sch.ctx.uc_stack.ss_sp, g_Sch.ctx.uc_stack.ss_size, &g_Sch);
        co_resume(&g_CoArray[i].m_co, &g_Sch);
    }
     processtask(result, idx);
    processtask(result, idx);
    printf("%d parent res : %d \n", co.index, result);
    return 0;
}


void testparent(bool isdynamic)
{    
    if (isdynamic)
    {
        g_Sch.allocfunc = malloc;
        g_Sch.freefunc = free;
        g_Sch.isDynamicStack = true;

        g_Sch.firststack = (char*)malloc(1024*128);
        g_Sch.firststackmaxsize = 1024*128;
        g_Sch.secondstack = (char*)malloc(1024*128);
        g_Sch.secondstackmaxsize = 1024*128;
    }
    


    for (int i = 0 ; i < MAX_CO_COUNT ; i+=100)
    {
        g_CoArray[i].index = i;
        g_CoArray[i].temp = i;
       co_init(&g_CoArray[i].m_co, processparent, &g_CoArray[i].index, g_CoArray[i].buff, sizeof(g_CoArray[i].buff), &g_Sch);
         //co_init(&g_CoArray[i].m_co, processparent, &g_CoArray[i].index, (char*)g_Sch.ctx.uc_stack.ss_sp, g_Sch.ctx.uc_stack.ss_size, &g_Sch);
        co_resume(&g_CoArray[i].m_co, &g_Sch);
    }
    while(1)
    {
        bool isAllEnd = true;
        for (int i = 0 ; i < MAX_CO_COUNT ; i+=1)
        {
            coroutine& co = g_CoArray[i].m_co;
            if(co.status != CO_STATE_END && g_CoArray[i].temp != -1)
            {
                isAllEnd = false;
                //printf("----------- : %d \n", i);
                //printf("find %d \n", idx);
                set_res(g_CoArray[i].temp);
                co_resume(&co, &g_Sch);
            }
        }
        
        if(isAllEnd)
        {
            break;
        }
    }
    return ;

    unsigned int t = (unsigned int)time(NULL);
    srand(t);

    while (1 )
    {
        bool isAllEnd = true;
        for (int i = 0 ; i < MAX_CO_COUNT ; ++i)
        {
            coroutine& co = g_CoArray[i].m_co;
            if(co.status != CO_STATE_END || g_CoArray[i].temp == -1)
            {
                isAllEnd = false;
                break;
            }
        }
        if(isAllEnd)
        {
            break;
        }

        int idx = rand()%MAX_CO_COUNT;
        coroutine& co = g_CoArray[idx].m_co;
        if(co.status != CO_STATE_END && g_CoArray[idx].temp != -1)
        {
            //printf("find %d \n", idx);
            set_res(g_CoArray[idx].temp);
            co_resume(&co, &g_Sch);
        }
    }
}

void testfunc(bool isdynamic)
{
    if (isdynamic)
    {
        g_Sch.allocfunc = malloc;
        g_Sch.freefunc = free;
        g_Sch.isDynamicStack = true;

        g_Sch.firststack = (char*)malloc(1024*128);
        g_Sch.firststackmaxsize = 1024*128;
        g_Sch.secondstack = (char*)malloc(1024*128);
        g_Sch.secondstackmaxsize = 1024*128;
    }
    


    for (int i = 0 ; i < MAX_CO_COUNT ; ++i)
    {
        g_CoArray[i].index = i;
        g_CoArray[i].temp = i;
        co_init(&g_CoArray[i].m_co, process, &g_CoArray[i].index, g_CoArray[i].buff, sizeof(g_CoArray[i].buff), &g_Sch);
    }
    
    for (int i = 0 ; i < MAX_CO_COUNT ; ++i)
    {
        //MyCoroutine& co = g_CoArray[i];
        co_resume(&g_CoArray[i].m_co, &g_Sch);
    }

    unsigned int t = (unsigned int)time(NULL);
    srand(t);

    while (1 )
    {
        bool isAllEnd = true;
        for (int i = 0 ; i < MAX_CO_COUNT ; ++i)
        {
            coroutine& co = g_CoArray[i].m_co;
            if(co.status != CO_STATE_END)
            {
                isAllEnd = false;
                break;
            }
        }
        if(isAllEnd)
        {
            break;
        }

        int idx = rand()%MAX_CO_COUNT;
        coroutine& co = g_CoArray[idx].m_co;
        if(co.status != CO_STATE_END)
        {
            set_res(g_CoArray[idx].temp);
            co_resume(&co, &g_Sch);
        }
    }
}



int main(int argc, const char *argv[])
{
    g_Sch.isDynamicStack = false;

    if (argc >=2 )
    {
        if (argv[1][0] == '1')
        {
            testperformance(false);
        }
        else if (argv[1][0] == '2')
        {
            testperformance1(false);
        }
        else if(argv[1][0] == 'f')
        {
            testfunc(false);
        }
        else if(argv[1][0] == 'p')
        {
            testparent(false);
        }
        else if (argv[1][0] == 'd' && argv[1][1] == 'f')
        {
            testfunc(true);
        }
        else if (argv[1][0] == 'd' && argv[1][1] == 'p')
        {
            testparent(true);
        }
        else if (argv[1][0] == 'd' && argv[1][1] == '1')
        {
            testperformance(true);
        }
        else if (argv[1][0] == 'd' && argv[1][1] == '2')
        {
            testperformance1(true);
        }
        return 0;
    }
    
    printf("lalala");

    
    //();
    return 0;
}

