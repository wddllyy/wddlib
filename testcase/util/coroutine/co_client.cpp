#include <iostream>
#include "stdio.h" // for printf
#include "sys/socket.h" //for socket
#include "fcntl.h" // for fcntl
#include "errno.h" // for errno
#include "string.h" //for strerror
#include "sys/epoll.h" // for epoll_*
#include "netinet/in.h"// for sockaddr_in
#include "arpa/inet.h"
#include <stdio.h>
#include <ucontext.h>
#include <unistd.h>
#include <memory.h>
#include <time.h>
#include <stdlib.h>
#include <map>
#include <pthread.h>
#include "util/coroutine/comgr.h"
using namespace std;

const int MaxSize = 64000;
const int MaxEps = 10240;
const int EventsSize = 10240;



SCoMgr mgr;


/* set file descriptor to non-block mode */
int SetNonBlock(int sock)
{
    // get sock fd flags;
    int flags = fcntl(sock, F_GETFL);
    if(flags == -1)
    {
        printf("fcntl get error(%d): %s/n", errno, strerror(errno));
        return -1;
    }
    //set fd's flag to non-block.
    flags |= O_NONBLOCK;
    flags = fcntl(sock, F_SETFL, flags);
    if(flags == -1)
    {
        printf("fcntl get error(%d): %s/n", errno, strerror(errno));
        return -2;
    }
    return 0;
}

int socket_send(int sockfd, const char* buffer, size_t buflen)
{
    int tmp;
    size_t total = buflen;
    const char *p = buffer;

    while(1)
    {
        tmp = send(sockfd, p, total, 0);
        if(tmp < 0)
        {
            // 当send收到信号时,可以继续写,但这里返回-1.
            if(errno == EINTR)
                return -1;

            // 当socket是非阻塞时,如返回此错误,表示写缓冲队列已满,
            // 在这里做延时后再重试.
            if(errno == EAGAIN)
            {
                printf("EAGAIN\n");
                usleep(1000);
                continue;
            }

            return -1;
        }

        if((size_t)tmp == total)
            return buflen;

        total -= tmp;
        p += tmp;
    }

    return tmp;
}

static char * g_pRes;
static int g_iResLen;

void SetRes(char * pRes, int iResLen)
{
    g_pRes = pRes;
    g_iResLen = iResLen;
}

void GetRes(char ** ppRes, int* iResLen)
{
    *ppRes = g_pRes;
    *iResLen = g_iResLen;
}
int rpc_get_res(int fd, char * pReq, int iReqLen, char ** ppRes, int* iResLen)
{
    socket_send(fd, pReq, iReqLen);
    int iret = mgr.Yield(time(NULL)+((fd%2) == 0 ? 10 : 10));
    if (iret < 0)
    {
        return iret;
    }
    GetRes(ppRes, iResLen);
    return 0;
}

int makemsg(char* sendbuf, int bufsize, int iCoPoolID, int num)
{
    *(int *)sendbuf = iCoPoolID;
    snprintf(sendbuf+sizeof(int), bufsize-sizeof(int), "I am a client, want to send data to server! %d ",num);
    return sizeof(int)+strlen(sendbuf+sizeof(int));
}
struct A
{
    A()
    {
        m_a = 0;
        memset(m_buf, 0 ,sizeof(m_buf));
    }
    int m_a;
    char m_buf[1024];
};
int process(int iCoPoolID)
{
    int i = 0;
    //A a;
    //a.m_a = 1;
    int iFD = *(int *)mgr.GetArg(iCoPoolID);
    //static int allcount = 0;
    while(1)
    {
        char  sendbuf [12800];
        int iret = makemsg(sendbuf, 128, iCoPoolID, i);
        //char  sendbuf [128]= {"aaaaaaaaaaaaaaaaaaaaaaaaaaaaa"};
        //*(int *)sendbuf = iCoPoolID;
        //int iret = 12;
        sendbuf[12700] = 'a';

        char * pRes ;
        int iResLen;
        iret = rpc_get_res(iFD, sendbuf, iret, &pRes, &iResLen);
        if (iret < 0)
        {
            printf("++++++ error :%d, %d ++++++\n",iFD, iret);
            return iret;
        }
        

        static int allcount = 0;
        if (allcount % 999999 == 0)
        {
            printf("stack size %d count %d\n", mgr.GetCo(iCoPoolID)->m_Co.stacksize, mgr.GeCoCount());
            
        }
        ++allcount;
        
        ++i;
    }
    return 0;
}
enum
{
    WM_COROUTINE = 0,
    WM_ASYN = 1,
    WM_THREAD = 2,
    WM_EPOLLTHREAD = 3,
};
int g_iWorkMode = 0;

struct thread_entry
{
    pthread_t thread;
    int sockfd;
    pthread_mutex_t mutex;
    int index;
};
thread_entry g_entrys[EventsSize];

void *pp(void *arg)
{

    thread_entry * pEntry = (thread_entry *)arg;
    int sockfd = pEntry->sockfd;


    while (1) 
    {
        char  sendbuf [12800];
        int iret = makemsg(sendbuf, 128, pEntry->index, sockfd);
        socket_send(sockfd, sendbuf, iret);

        int n;
        if (g_iWorkMode == WM_EPOLLTHREAD)
        {
            pthread_mutex_lock(&pEntry->mutex);
        }
        else
        {
            n = read(sockfd, sendbuf, 128);
            if( n == 0)
            {
                printf("close socket %d, read = 0", sockfd);
                close(sockfd);
            }
            else if(n < 0)
            {
                if(errno == ECONNRESET)
                {
                    printf("close socket %d, read < 0", sockfd);
                    close(sockfd);
                }
                else
                {
                    printf("read failed");
                }
            }
        }



    }
    return NULL;
}

/* main entrance */
int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        fprintf (stderr, "Usage: %s [port] [mode]\n mode: c -> coroutine; a -> asyn; \n", argv[0]);
        exit (EXIT_FAILURE);
    }

    

    int port = atoi(argv[1]);
    struct sockaddr_in clientAddr;
    bzero(&clientAddr, sizeof(clientAddr));
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons(port);
    clientAddr.sin_addr.s_addr = inet_addr("127.0.0.1");


    if (argv[2][0] == '0')
    {
        g_iWorkMode = WM_COROUTINE;
        mgr.InitCoMgr(sizeof(int), 128*1024, EventsSize,  FIXED_STACKSIZE_POOL, NULL, NULL);
    }
    else if (argv[2][0] == '1')
    {
        g_iWorkMode = WM_COROUTINE;
        mgr.InitCoMgr(sizeof(int), 128*1024, EventsSize,  FIXED_STACKSIZE_MALLOC, malloc, free);
    }
    else if (argv[2][0] == '2')
    {
        g_iWorkMode = WM_COROUTINE;
        mgr.InitCoMgr(sizeof(int), 0 , EventsSize, DYNAMIC_STACKSIZE_MALLOC, malloc, free);
    }
    else if (argv[2][0] == 'a')
    {
        g_iWorkMode = WM_ASYN;
    }
    else if (argv[2][0] == 't')
    {
        g_iWorkMode = WM_THREAD;
        for (int i = 0 ; i < EventsSize; ++i)
        {
            int sockfd = socket(AF_INET, SOCK_STREAM, 0);
            if(sockfd == -1)
            {
                printf("create socket error...errno[%d]\n",errno);
                continue;
            }
            //connect
            int ret = connect(sockfd, (sockaddr*)&clientAddr, sizeof(sockaddr_in));
            if (ret != 0)
            {
                printf("connect socket error...errno[%d]\n",errno);
                continue;
            }
            g_entrys[i].sockfd = sockfd;
        }
        

        for (int i = 0 ; i < EventsSize; ++i)
        {   
            pthread_create(&g_entrys[i].thread, NULL, pp, (void *)&g_entrys[i]);
        }
        
        while(1)
        {
            printf("I am main thread\n");
            pthread_join(g_entrys[1].thread, NULL);
        }
        return 0;
    }
    else if (argv[2][0] == 'm')
    {
        g_iWorkMode = WM_EPOLLTHREAD;
    }

    


    //create epoll fd
    int epfd = epoll_create(MaxEps);
    //add fd to epfd
    struct epoll_event events[EventsSize];

    for (int i = 0 ; i < EventsSize; i ++)
    {
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if(sockfd == -1)
        {
            printf("create socket error...errno[%d]\n",errno);
            continue;
        }
        //connect
        int ret = connect(sockfd, (sockaddr*)&clientAddr, sizeof(sockaddr_in));
        if(ret == 0)
        {
            SetNonBlock(sockfd);
            events[i].data.fd = sockfd;
            events[i].events = EPOLLERR | EPOLLIN;
            epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &events[i]);
        }
        else
        {
            events[i].data.fd = -1;
        }

        if (g_iWorkMode == WM_COROUTINE )
        {
            int iRet = mgr.Alloc(process, &sockfd, sizeof(int));
            mgr.Resume(iRet);
        }
        else if (g_iWorkMode == WM_ASYN)
        {
            char  sendbuf [128]= {0};
            int iret = makemsg(sendbuf, 128, sockfd, 123);
            write(sockfd, sendbuf, iret);
        }
        else if(g_iWorkMode == WM_EPOLLTHREAD)
        {
            g_entrys[i].sockfd = sockfd;
            g_entrys[i].index = i;
            pthread_create(&g_entrys[i].thread, NULL, pp, (void *)&g_entrys[i]);
        }
        

    }

    //begin to accept
    printf("begin to communication...\n");
    char buf[MaxSize];
    for(;;)
    {   
        //sleep(1);
        int nfds = epoll_wait(epfd, events, EventsSize, 2000);
        if(nfds == -1)
        {
            if(errno == EINTR) continue;
        }
        if (nfds == 0)
        {
            mgr.ProcessAllExpire(time(NULL));
            int count = mgr.GeCoCount();
            printf("left count : %d...\n", count);
            if (count == 0)
            {
                
                break;
            }
            
        }
        

        for(int i = 0; i < nfds; i++)
        {
            if ((events[i].events & EPOLLERR) ||
                (events[i].events & EPOLLHUP)
                )
            {
                /* An error has occured on this fd, or the socket is not
                ready for reading (why were we notified then?) */
                fprintf (stderr, "epoll error  %d\n", events[i].data.fd);
                close (events[i].data.fd);
                continue;
            }
            else if(events[i].events & EPOLLIN)
            {
                int sockfd = events[i].data.fd;
                if(sockfd < 0) continue;

                //read
                int n = read(sockfd, buf, MaxSize);
                if( n == 0)
                {
                    printf("close socket %d, read = 0", sockfd);
                    close(sockfd);
                    events[i].data.fd = -1;
                }
                else if(n < 0)
                {
                    if(errno == ECONNRESET)
                    {
                        printf("close socket %d, read < 0", sockfd);
                        close(sockfd);
                        events[i].data.fd = -1;
                    }
                    else
                        printf("read failed");
                }
                buf[n]=0;
                SetRes(buf, n);

                int iID = *(int *)buf;
                
                if (g_iWorkMode == WM_COROUTINE )
                {
                    mgr.Resume(iID);
                }
                else if (g_iWorkMode == WM_ASYN)
                {
                    char  sendbuf [128]= {0};
                    int iret = makemsg(sendbuf, 128, iID, 123);
                    write(iID, sendbuf, iret);
                }
                else if (g_iWorkMode == WM_EPOLLTHREAD)
                {
                    pthread_mutex_unlock(&g_entrys[iID].mutex);
                }
            }
        }

        if (g_iWorkMode == WM_COROUTINE )
        {
            mgr.ProcessPartExpire(time(NULL), EventsSize/10);
        }
    }
    for( int i = 0; i < EventsSize; i++)
    {
        int sockfd = events[i].data.fd;
        if( sockfd > 0)
        {
            close(sockfd);
            events[i].data.fd = -1;
        }
    }

    mgr.Destory();
    return 0;
}

