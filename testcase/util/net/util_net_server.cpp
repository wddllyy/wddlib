#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>

#include "util/net/epoll_poller.h"
#include "util/net/tcp_server.h"
#include "testcase_util.h"
#include "util/log/logmgr.h"
struct timeval starttime ;


class MyServer : public TcpServer
{
public:
    MyServer(EPollPoller& poll, InetAddress addr, bool isReUsePort)
        : TcpServer(poll, addr, isReUsePort)
    {

    }
    virtual int OnMsgRecv(ServerChannel& channel)
    {
        //printf("Recv %d bytes Msg %s ", (int)channel.ReadableBytes(), channel.PeekReadBuf() );

        channel.SendMsg(channel.PeekReadBuf(), channel.ReadableBytes());
        channel.RetrieveReadBuf(channel.ReadableBytes());
        static int successcount = 0;
        successcount++;
        if (successcount > 100000 )
        {
            struct timeval now ;
            gettimeofday(&now, NULL);

            float diff =   (now.tv_sec-starttime.tv_sec) + ((now.tv_usec - starttime.tv_usec))/1000000.f;
            printf("count %d, tps %f\n",successcount, (float)successcount / diff);
            starttime = now;
            successcount = 1;
        }

        return 0;
    }
    virtual int OnNewChannel(int iFD, InetAddress addr)
    {
        printf("OnNewChannel %d from %s count:%d\n", iFD, addr.ToIpPort(), (int)m_ChannelMap.size());
        return TcpServer::OnNewChannel(iFD, addr);
    }
    virtual int OnCloseChannel( ServerChannel& channel )
    {
        printf("OnCloseChannel %d from %s count:%d\n", channel.GetFD(), channel.GetPeerAddr().ToIpPort(), (int)m_ChannelMap.size() );
        return TcpServer::OnCloseChannel(channel);
    }
    
};

int util_server_test(int argc, char *argv[])
{
    GetDefaultLogMgr().AddFileCat(LOGLV_FATAL, LOGLV_FATAL, 20*1024*1024, 5, "server", "log");


    EPollPoller poll;
    poll.InitEpoll();

    MyServer server(poll, InetAddress("0.0.0.0", 7789), true);
    //client.Connect();
    server.Start();

    
    gettimeofday(&starttime, NULL);

    while(1)
    {
        poll.Poll(1000);
        //printf("poll\n");
    }


    return 0;
}


int main(int argc, char* argv[])
{
    util_server_test(argc, argv);
    return 0;
}