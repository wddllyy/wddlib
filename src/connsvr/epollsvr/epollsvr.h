#ifndef __CONNSVR_EPOLLSVR_EPOLLSVR_H__
#define __CONNSVR_EPOLLSVR_EPOLLSVR_H__

#include "util/net/epoll_poller.h"
#include "util/net/tcp_server.h"
#include "util/log/logmgr.h"
#include "util/singleton/Singleton.h"



extern EPollPoller __unvisiable_poll;

class EpollServer : public TcpServer
{
public:
    EpollServer( InetAddress addr, bool isReUsePort)
        : TcpServer(__unvisiable_poll, addr, isReUsePort)
    {

    }
    virtual int OnMsgRecv(ServerChannel& channel);
    virtual int OnNewChannel(int iFD, InetAddress addr);
    virtual int OnCloseChannel( ServerChannel& channel );
};

typedef SingletonHolder<EpollServer> SEpollServer;

#endif

