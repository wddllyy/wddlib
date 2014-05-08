#include <fcntl.h>
#include <sys/socket.h>
#include <stdint.h>
#include <endian.h>
#include <errno.h>
#include <stdio.h>  // snprintf
#include <strings.h>  // bzero
#include <unistd.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include "util/net/tcp_socket.h"
#include "util/net/epoll_poller.h"
#include "util/log/logmgr.h"


TcpSocket::TcpSocket(EPollPoller& poll)
    :m_iFD(-1)
    ,m_EPoll(poll)
    ,m_iEvents(kNoneEvent)
    ,m_isPooling(false)
{
    
}


int TcpSocket::SetNonBlock()
{
    int flags = ::fcntl(m_iFD, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(m_iFD, F_SETFL, flags);
    return ret;
}

int TcpSocket::SetReuseAddr( bool on )
{
    int optval = on ? 1 : 0;
    ::setsockopt(m_iFD, SOL_SOCKET, SO_REUSEADDR, &optval, static_cast<socklen_t>(sizeof optval));
    return 0;
}

int TcpSocket::SetReusePort( bool on )
{
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    int ret = ::setsockopt(m_iFD, SOL_SOCKET, SO_REUSEPORT,
        &optval, static_cast<socklen_t>(sizeof optval));
    return ret;
#else
    LOG_ERROR("Not support SetReusePort");
    return -1;
#endif
}

int TcpSocket::SetTcpNoDelay(bool on)
{
    int optval = on ? 1 : 0;
    return ::setsockopt(m_iFD, IPPROTO_TCP, TCP_NODELAY,
        &optval, static_cast<socklen_t>(sizeof optval));
}

int TcpSocket::SetKeepAlive( bool on )
{
    int optval = on ? 1 : 0;
    return ::setsockopt(m_iFD, SOL_SOCKET, SO_KEEPALIVE,
        &optval, static_cast<socklen_t>(sizeof optval));
}

//int TcpSocket::ShutdownWrite()
//{
//    int iRet = ::shutdown(m_iFD, SHUT_WR);
//    if (iRet < 0)
//    {
//        LOG_ERROR("sockets::shutdownWrite ERROR");
//    }
//    return iRet;
//}





int TcpSocket::Close()
{
    DisableAll();
    int iRet = ::close(m_iFD);
    if (iRet < 0)
    {
        LOG_ERROR("sockets::close errorcode :%d", iRet);
    }
    return iRet;
}


void TcpSocket::UpdateEvent()
{
    m_EPoll.UpdateEvent(this);
}

int TcpSocket::GetSocketError()
{
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(m_iFD, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0)
    {
        return errno;
    }
    else
    {
        return optval;
    }
}

int TcpSocket::HandleError()
{
    int errorcode = GetSocketError();
    LOG_ERROR("sockets::readv ERROR %d", errorcode);
    return 0;
}

int TcpSocket::CreateNoBlock()
{
    int sockfd = ::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sockfd < 0)
    {
        int errorcode = errno;
        LOG_ERROR("CreateNoBlock fail errorcode : %d", errorcode);
        return -1;
    }
    m_iFD = sockfd;
    return 0;
}

void TcpSocket::SetPooling( bool on )
{
    m_isPooling = on;
}

bool TcpSocket::IsPooling()
{
    return m_isPooling;
}

TcpSocket::~TcpSocket()
{

}

int TcpSocket::SetFD( int iFD )
{
    m_iFD = iFD;
    return 0;
}

InetAddress TcpSocket::GetLocalAddr()
{
    InetAddress addr;
    socklen_t addrlen = static_cast<socklen_t>(sizeof(sockaddr_in));
    if (::getsockname(m_iFD, (sockaddr*) &(addr.GetSockAddrInet()), &addrlen) < 0)
    {
        int errorcode = errno;
        LOG_ERROR("GetLocalAddr fail errorcode : %d", errorcode);
    }
    return addr;
}

InetAddress TcpSocket::GetPeerAddr()
{
    InetAddress addr;
    socklen_t addrlen = static_cast<socklen_t>(sizeof(sockaddr_in));
    if (::getpeername(m_iFD, (sockaddr*) &(addr.GetSockAddrInet()), &addrlen) < 0)
    {
        int errorcode = errno;
        LOG_ERROR("GetPeerAddr fail errorcode : %d", errorcode);
    }
    return addr;
}

InetAddress::InetAddress( uint16_t port )
{
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_addr.s_addr = htobe32(INADDR_ANY);
    addr_.sin_port = htobe16(port);
}

InetAddress::InetAddress( const char * ip, uint16_t port )
{
    bzero(&addr_, sizeof addr_);
    addr_.sin_family = AF_INET;
    addr_.sin_port = htobe16(port);
    if (::inet_pton(AF_INET, ip, &addr_.sin_addr) <= 0)
    {
        LOG_ERROR("sockets::ip error : %s", ip);
    }
}

InetAddress::InetAddress( const struct sockaddr_in& addr )
    : addr_(addr)
{
    bzero(&addr_, sizeof addr_);
}

InetAddress::InetAddress()
{

}

const char * InetAddress::ToIp() const
{
    static char buf[32];
    memcpy(buf, "INVALID", 8);
    if(::inet_ntop(AF_INET, &addr_.sin_addr, buf, sizeof(buf)) == NULL)
    {
        LOG_ERROR("sockets::ip error");
        return buf;
    }
    return buf;
}

const char * InetAddress::ToIpPort() const
{
    static char buf[32];
    memcpy(buf, "INVALID", 8);
    uint16_t port = be16toh(addr_.sin_port);
    snprintf(buf, sizeof(buf), "%s:%u", ToIp(), port);
    return buf;
}
