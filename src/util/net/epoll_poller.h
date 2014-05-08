#ifndef __UTIL_EPOLL_POLLER_H
#define __UTIL_EPOLL_POLLER_H

#include <vector>
#include <util/net/tcp_socket.h>

class TcpSocket;

class EPollPoller
{
public:
    EPollPoller();
    ~EPollPoller();
public:
    int InitEpoll();
    void UpdateEvent(TcpSocket * pSocket);
    int Poll(int timeoutMs);
    
protected:
    

    
private:
    int HandleEvent(TcpSocket* pSocket, int iEvent);
    int Update(int op, TcpSocket* pSocket);

    typedef std::vector<struct epoll_event> EventList;
    static const int kInitEventListSize = 16;

    EventList m_EventList;
    int m_EpollFD;

};
#endif
