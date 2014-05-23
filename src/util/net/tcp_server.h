#ifndef __UTIL_TCP_SERVER_H
#define __UTIL_TCP_SERVER_H

#include <map>
#include "util/net/tcp_channel.h"
#include "util/net/epoll_poller.h"

class TcpServer;
class TcpAcceptor : public TcpSocket
{
public:
    TcpAcceptor(TcpServer & server, InetAddress addr, bool isReUsePort);
    ~TcpAcceptor();

    int Listen();
    int Accept(InetAddress & peerAddr);

    virtual int HandleCanRecv();
    virtual int HandleCanSend();
    virtual int HandleError();
    virtual int HandleClose();
private:
    TcpServer & m_Server;
    InetAddress m_ListenAddr;
    bool m_isReUsePort;
    int m_iIdleFD;
};

class ServerChannel : public TcpChannel
{
public:
    ServerChannel(TcpServer & server);
    ~ServerChannel();

    int Disconnect();
    int AcceptedFD(int iFD);

    virtual int HandleCanRecv();
    virtual int HandleCanSend();
    virtual int HandleError();
    virtual int HandleClose();
    virtual int OnRecvMsg();

    int SendMsg(const char * data, size_t len);
public:
    TcpServer & m_Server;
};



class TcpServer
{
public:
    TcpServer(EPollPoller& poll, InetAddress addr, bool isReUsePort);
    virtual ~TcpServer();
public:
    int Start();
    int Finish();
    EPollPoller & GetEPoll(){return m_EPoll;}


    virtual int OnNewChannel(int iFD, InetAddress addr);
    virtual int OnMsgRecv( ServerChannel& channel );
    virtual int OnCloseChannel( ServerChannel& channel );
protected:
    
protected:
    typedef std::map<int,ServerChannel*> ChannleMap;
    EPollPoller& m_EPoll;
    TcpAcceptor m_Acceptor;
    ChannleMap m_ChannelMap;
    

};



#endif
