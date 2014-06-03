#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "util/net/epoll_poller.h"
#include "util/net/tcp_client.h"
#include "testcase_util.h"
#include "util/log/logmgr.h"

class MyClient : public TcpClient
{
public:
    MyClient(EPollPoller& poll, InetAddress addr)
        : TcpClient(poll, addr)
    {

    }
    virtual int OnRecvMsg()
    {
        printf("Recv %d bytes Msg %s ", (int)m_RecvBuf.ReadableBytes(), m_RecvBuf.Peek() );
        m_RecvBuf.Retrieve(m_RecvBuf.ReadableBytes());

        
        return 0;
    }
    
};

int util_client_test(int argc, char *argv[])
{
    GetDefaultLogMgr().AddFileCat(LOGLV_TRACE, LOGLV_FATAL, 20*1024*1024, 5, "client", "log");


    EPollPoller poll;
    poll.InitEpoll();

    MyClient client(poll, InetAddress("10.24.250.73", 7789));
    client.Connect();

    while(1)
    {
        sleep(1);
        poll.Poll(1000);

    }
    /*
    client.SendMsg("start", 7);
    while(1)
    {
        sleep(1);
        poll.Poll(1000);
        
        printf("poll\n");
        static char buf [1024];
        static int idx = 0;
        snprintf(buf, 1024, "hello:%d", idx++);
        client.SendMsg(buf, strlen(buf)+1);
        printf("write buf size %d ", (int)client.GetWriteBufBytes());
    }
*/

    return 0;
}
class MyClient1 : public TcpClient
{
public:
    MyClient1(EPollPoller& poll, InetAddress addr)
        : TcpClient(poll, addr)
    {

    }
    virtual int OnRecvMsg()
    {
        //printf("FD:%d Recv %d bytes Msg %s \n", m_iFD, (int)m_RecvBuf.ReadableBytes(), m_RecvBuf.Peek() );
        m_RecvBuf.Retrieve(m_RecvBuf.ReadableBytes());

        static char buf [1024];
        static int idx = 0;
        snprintf(buf, 1024, "hello:%d", idx++);
        SendMsg(buf, strlen(buf)+1);
        
        return 0;
    }

};

int util_client_test1(int argc, char *argv[])
{
    GetDefaultLogMgr().AddFileCat(LOGLV_FATAL, LOGLV_FATAL, 20*1024*1024, 5, "client", "log");


    EPollPoller poll;
    poll.InitEpoll();

    for(int i = 0; i < 10000; ++i)
    {
        MyClient1 *pClient = new MyClient1(poll, InetAddress("10.24.250.73", 7789));
        pClient->SendMsg("hello", 6);
    }



    
    while(1)
    {
        poll.Poll(1000);
    }


    return 0;
}

int main(int argc, char* argv[])
{
    util_client_test(argc, argv);
    return 0;
}