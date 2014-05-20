#ifndef __UTIL_TCP_CLIENT_H
#define __UTIL_TCP_CLIENT_H

#include "util/net/tcp_channel.h"
#include "util/net/epoll_poller.h"


class TcpClient : public TcpChannel
{
public:
    enum States { kDisconnected, kConnecting, kConnected };

    TcpClient(EPollPoller& poll, InetAddress addr);
    ~TcpClient();

    
public:
    // operator
    int Connect();
    int Disconnect();
    virtual int OnRecvMsg();
    int SendMsg(const char * data, size_t len);

    // callback
    virtual int HandleCanRecv();
    virtual int HandleCanSend();
    virtual int HandleError();
    virtual int HandleClose();
    
    InetAddress GetPeerAddr(){return m_PeerAddr;}

private:
    int _Connected();

private:
     States m_state;
     bool m_isAutoReconnect;
     InetAddress m_PeerAddr;
};

#endif
