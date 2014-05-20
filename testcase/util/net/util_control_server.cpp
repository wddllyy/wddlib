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


class ControlServer : public TcpServer
{
public:
    ControlServer(EPollPoller& poll, InetAddress addr, bool isReUsePort)
        : TcpServer(poll, addr, isReUsePort)
    {

    }
    virtual int OnMsgRecv(ServerChannel& channel)
    {
        printf("Recv %d bytes Msg %s ", (int)channel.ReadableBytes(), channel.PeekReadBuf() );
        channel.SendMsg(channel.PeekReadBuf(), channel.ReadableBytes());

        
        channel.RetrieveReadBuf(channel.ReadableBytes());
        
        
        

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
    GetDefaultLogMgr().AddFileCat(LOGLV_FATAL, LOGLV_FATAL, 20*1024*1024, 5, "control_server", "log");


    EPollPoller poll;
    poll.InitEpoll();

    ControlServer server(poll, InetAddress("0.0.0.0", 7777), true);
    server.Start();

    
    gettimeofday(&starttime, NULL);

    while(1)
    {
        poll.Poll(1000);
    }


    return 0;
}


int main(int argc, char* argv[])
{
    util_server_test(argc, argv);
    return 0;
}