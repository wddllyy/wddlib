#ifndef __TESTCONNSVR_EPOLLSVR_EPOLLSVR_H__
#define __TESTCONNSVR_EPOLLSVR_EPOLLSVR_H__

#include "util/net/epoll_poller.h"
#include "util/net/tcp_server.h"
#include "util/net/tcp_client.h"
#include "util/log/logmgr.h"
#include "util/singleton/Singleton.h"
#include "connsvrtest/framework/testconnsvr.h"
#include <vector>

class EpollServer : public TcpServer
{
public:
	typedef std::vector<int> IDXVec;
    EpollServer( InetAddress addr, bool isReUsePort)
		: TcpServer(G_ConnSvr.GetPoll(), addr, isReUsePort)
    {
	
    }
    virtual int OnMsgRecv(ServerChannel& channel);
    virtual int OnNewChannel(int iFD, InetAddress addr);
    virtual int OnCloseChannel( ServerChannel& channel );

protected:
};

typedef SingletonHolder<EpollServer> SEpollServer;



#endif

