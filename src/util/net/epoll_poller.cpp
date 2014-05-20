#include <sys/epoll.h>
#include <errno.h>
#include "util/net/epoll_poller.h"
#include "util/log/logmgr.h"


EPollPoller::EPollPoller()
    :m_EventList(kInitEventListSize)
{

}

EPollPoller::~EPollPoller()
{

}

int EPollPoller::InitEpoll()
{
    m_EpollFD = ::epoll_create1(EPOLL_CLOEXEC);
    if (m_EpollFD < 0)
    {
        int err = errno;
        LOG_ERROR("Connector::InitEpoll Error - %d %s", err, strerror(err));
        return -1;
    }
    return 0;
}

void EPollPoller::UpdateEvent( TcpSocket * pSocket )
{
    bool isPooling = pSocket->IsPooling();
    if (!isPooling)
    {
        if (pSocket->IsNoneEvent())
        {
            return;
        }
        Update(EPOLL_CTL_ADD, pSocket);
        pSocket->SetPooling(true);
    }
    else
    {
        if (pSocket->IsNoneEvent())
        {
            Update(EPOLL_CTL_DEL, pSocket);
            pSocket->SetPooling(false);
        }
        else
        {
            Update(EPOLL_CTL_MOD, pSocket);
        }
    }
}

int EPollPoller::Update( int op, TcpSocket* pSocket )
{
    struct epoll_event event;
    bzero(&event, sizeof event);
    event.events = pSocket->GetEpollEvent();
    event.data.ptr = pSocket;
    int fd = pSocket->GetFD();
    LOG_TRACE("EPOLL_CTL_ADD op:%d fd:%d", op, pSocket->GetFD());
    int iRet = ::epoll_ctl(m_EpollFD, op, fd, &event);
    if (iRet < 0)
    {
        int errorcode = errno;
        if (op == EPOLL_CTL_DEL)
        {
            LOG_ERROR("epoll_ctl op= %d fd=%d error:%d %s", op, fd, errorcode, strerror(errorcode));
        }
        else
        {
            LOG_FATAL("epoll_ctl op= %d fd=%d error:%d %s", op, fd, errorcode, strerror(errorcode));
        }
        return -1;
    }
    return 0;
}

int EPollPoller::Poll(int timeoutMs)
{
    int numEvents = ::epoll_wait(m_EpollFD,
        &*m_EventList.begin(),
        static_cast<int>(m_EventList.size()),
        timeoutMs);
    int savedErrno = errno;
    if (numEvents > 0)
    {
        LOG_TRACE("%d events happended", numEvents);
        if (static_cast<size_t>(numEvents) == m_EventList.size())
        {
            m_EventList.resize(m_EventList.size()*2);
        }
    }
    else if(numEvents < 0)
    {
        if (savedErrno != EINTR)
        {
            LOG_ERROR("Connector::InitEpoll Error - %d %s", savedErrno, strerror(savedErrno));
        }
        return -1;
    }

    for (int i = 0; i < numEvents; ++i)
    {
        TcpSocket* pSocket = static_cast<TcpSocket*>(m_EventList[i].data.ptr);
        HandleEvent(pSocket, m_EventList[i].events);
    }
    return 0;
}

int EPollPoller::HandleEvent( TcpSocket* pSocket, int iEvent)
{
    bool isError = false;
    if ((iEvent & EPOLLHUP) && !(iEvent & EPOLLIN))
    {
        LOG_TRACE("HandleClose- %d %d", pSocket->GetFD(), iEvent);
        pSocket->HandleClose();
        isError = true;
    }
    else if (iEvent & (EPOLLERR))
    {
        LOG_TRACE("HandleError- %d %d", pSocket->GetFD(), iEvent);
        pSocket->HandleError();
        isError = true;
    }

    if (iEvent & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
    {
        LOG_TRACE("HandleCanRecv- %d %d", pSocket->GetFD(), iEvent);
        pSocket->HandleCanRecv();
    }

    if (!isError && iEvent & EPOLLOUT)
    {
        LOG_TRACE("HandleCanSend- %d %d", pSocket->GetFD(), iEvent);
        pSocket->HandleCanSend();
    }
    return 0;
}
