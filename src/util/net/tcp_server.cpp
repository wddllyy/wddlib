#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include "util/log/logmgr.h"
#include "util/net/tcp_server.h"




TcpAcceptor::TcpAcceptor( TcpServer & server, InetAddress addr, bool isReUsePort )
    :TcpSocket(server.GetEPoll())
    ,m_Server(server)
    ,m_ListenAddr(addr)
    ,m_isReUsePort(isReUsePort)
    ,m_iIdleFD(::open("/dev/null", O_RDONLY | O_CLOEXEC))
{

}

TcpAcceptor::~TcpAcceptor()
{

}

int TcpAcceptor::Listen()
{
    int ret = CreateNoBlock();
    if (ret < 0)
    {
        return -1;
    }
    SetReuseAddr(true);
    SetReusePort(m_isReUsePort);
    ret = ::bind(m_iFD, (sockaddr*) &(m_ListenAddr.GetSockAddrInet()), sizeof(sockaddr_in));
    if (ret < 0)
    {
        int errorcode = errno;
        LOG_ERROR("bind error ret:%d error:%d %s", ret, errorcode, strerror(errorcode));
        return -1;
    }

    ret = ::listen(m_iFD, SOMAXCONN);
    if (ret < 0)
    {
        int errorcode = errno;
        LOG_ERROR("listen error ret:%d error:%d %s", ret, errorcode, strerror(errorcode));
        return -2;
    }

    EnableReading();
    return ret;
}

int TcpAcceptor::HandleCanRecv()
{
    InetAddress peerAddr;

    //FIXME loop until no more
    int connfd = Accept(peerAddr);
    if (connfd >= 0)
    {
        m_Server.OnNewChannel(connfd, peerAddr);
    }
    else
    {
        LOG_ERROR("in Acceptor::handleRead");
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of livev.
        if (errno == EMFILE)
        {
            ::close(m_iIdleFD);
            m_iIdleFD = ::accept(m_iFD, NULL, NULL);
            ::close(m_iIdleFD);
            m_iIdleFD = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
    return 0;
}

int TcpAcceptor::Accept(InetAddress & peerAddr)
{
    socklen_t addrlen = static_cast<socklen_t>(sizeof(sockaddr_in));
    int connfd = ::accept4(m_iFD, (sockaddr*) &(peerAddr.GetSockAddrInet()), &addrlen, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (connfd < 0)
    {
        int savedErrno = errno;
        LOG_ERROR("Socket::accept");
        switch (savedErrno)
        {
        case EAGAIN:
        case ECONNABORTED:
        case EINTR:
        case EPROTO: // ???
        case EPERM:
        case EMFILE: // per-process lmit of open file desctiptor ???
            // expected errors
            errno = savedErrno;
            break;
        case EBADF:
        case EFAULT:
        case EINVAL:
        case ENFILE:
        case ENOBUFS:
        case ENOMEM:
        case ENOTSOCK:
        case EOPNOTSUPP:
            // unexpected errors
            LOG_FATAL("unexpected error of ::accept listen_fd:%d error:%d", m_iFD, savedErrno);
            break;
        default:
            LOG_FATAL("unknown error of ::accept listen_fd:%d error:%d", m_iFD, savedErrno);
            break;
        }
    }
    return connfd;
}

int TcpAcceptor::HandleCanSend()
{
    LOG_FATAL("Acceptor unexpected HandleCanSend error of ::accept listen_fd:%d", m_iFD);
    return -1;
}

int TcpAcceptor::HandleError()
{
    LOG_FATAL("Acceptor HandleError of ::accept listen_fd:%d", m_iFD);
    return -1;
}

int TcpAcceptor::HandleClose()
{
    DisableAll();
    Close();
    LOG_FATAL("Acceptor HandleClose of ::accept listen_fd:%d", m_iFD);
    return -1;
}


ServerChannel::ServerChannel( TcpServer & server )
    :TcpChannel(server.GetEPoll())
    ,m_Server(server)
{

}

ServerChannel::~ServerChannel()
{

}

int ServerChannel::HandleCanRecv()
{
    return TcpChannel::DoRecv();
}

int ServerChannel::HandleCanSend()
{
    return DoContineSend();
}

int ServerChannel::HandleError()
{
    int errorcode = errno;
    int ec = GetSocketError();
    LOG_ERROR("TcpClient::HandleError ERROR %d %s  -- GetSocketError %d %s", errorcode, strerror(errorcode), ec, strerror(ec));
    return 0;
}

int ServerChannel::HandleClose()
{
    LOG_ERROR("HandleClose of fd:%d", m_iFD);
    DisableAll();
    Close();
    return m_Server.OnCloseChannel(*this);
}

int ServerChannel::Disconnect()
{
    return HandleClose();
}

int ServerChannel::OnRecvMsg()
{
    return m_Server.OnMsgRecv(*this);
}

int ServerChannel::AcceptedFD( int iFD )
{
    TcpSocket::SetFD(iFD);
    EnableReading();
    return 0;
}

int ServerChannel::SendMsg( const char * data, size_t len )
{
    return TcpChannel::DoSend(data, len);
}




TcpServer::TcpServer( EPollPoller& poll, InetAddress addr, bool isReUsePort )
    :m_EPoll(poll)
    ,m_Acceptor(*this, addr, isReUsePort)
{

}

TcpServer::~TcpServer()
{

}


int TcpServer::Start()
{
    return m_Acceptor.Listen();
}

int TcpServer::Finish()
{
    m_Acceptor.HandleClose();
    for (ChannleMap::iterator ite = m_ChannelMap.begin(); ite != m_ChannelMap.end(); ++ite)
    {
        ChannleMap::iterator itenow = ite;
        itenow->second->Disconnect();
    }
    return 0;
}

int TcpServer::OnNewChannel(int iFD, InetAddress addr)
{
    ChannleMap::iterator ite = m_ChannelMap.find(iFD);
    if (ite != m_ChannelMap.end())
    {
        ite->second->Disconnect();
    }
    ServerChannel* pChannel = new ServerChannel(*this);
    m_ChannelMap[iFD] = pChannel;
    pChannel->AcceptedFD(iFD);

    return 0;
}

int TcpServer::OnMsgRecv( ServerChannel& channel )
{
    return 0;
}

int TcpServer::OnCloseChannel( ServerChannel& channel )
{
    int iFD = channel.GetFD();
    delete &channel;
    m_ChannelMap.erase(iFD);
    return 0;
}
