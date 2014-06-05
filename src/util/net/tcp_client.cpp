#include <errno.h>
#include "util/net/tcp_client.h"
#include "util/log/logmgr.h"



TcpClient::TcpClient(EPollPoller& poll, InetAddress serveraddr)
    :TcpChannel(poll)
    ,m_state(kDisconnected)
    ,m_isAutoReconnect(true)
    ,m_PeerAddr(serveraddr)
{
    CreateNoBlock();
}

TcpClient::~TcpClient()
{

}

int TcpClient::Connect( )
{
    if (m_state != kDisconnected)
    {
        return 1;
    }

    m_isAutoReconnect = true;
    DisableAll();
    int ret = CreateNoBlock();
    if (ret < 0)
    {
        return -1;
    }
    ret = ::connect(m_iFD, (sockaddr*) &(m_PeerAddr.GetSockAddrInet()), sizeof(sockaddr_in));
    int savedErrno = (ret == 0) ? 0 : errno;
    switch (savedErrno)
    {
    case 0:
    case EINPROGRESS:
    case EINTR:
    case EISCONN:
        LOG_TRACE("kConnecting %d", m_iFD);
        m_state = kConnecting;
        EnableWriting();
        ret = 0;
        break;

    case EAGAIN:
    case EADDRINUSE:
    case EADDRNOTAVAIL:
    case ECONNREFUSED:
    case ENETUNREACH:
        LOG_TRACE("kDisconnected %d", m_iFD);
        m_state = kDisconnected;
        ret = 0;
        break;

    case EACCES:
    case EPERM:
    case EAFNOSUPPORT:
    case EALREADY:
    case EBADF:
    case EFAULT:
    case ENOTSOCK:
        LOG_ERROR("connect error %d", savedErrno);
        DoClose();
        ret = -1;
        break;

    default:
        LOG_ERROR("Unexpected error in Connect %d", savedErrno);
        DoClose();
        ret = -1;
        break;
    }

    return ret;
}

int TcpClient::Disconnect()
{
    m_isAutoReconnect = false;
    DoClose();
    return 0;
}

int TcpClient::HandleCanRecv()
{
    return TcpChannel::DoRecv();
}

int TcpClient::HandleCanSend()
{
    LOG_ERROR("HandleCanSend %d", m_iFD);
    if (m_state == kConnecting)
    {
        LOG_ERROR("connected %d", m_iFD);
        return _Connected();
    }
    else if (m_state == kConnected)
    {
        return DoContineSend();
    }
    else
    {
        LOG_ERROR("kDisconnected and cansend ERROR %d", m_iFD);
        return -1;
    }
}

int TcpClient::HandleError()
{
    int errorcode = errno;
    LOG_ERROR("TcpClient::HandleError ERROR %d %s", errorcode, strerror(errorcode));
    return 0;
}

int TcpClient::HandleClose()
{
    if (m_state == kConnecting)
    {
        int err = GetSocketError();
        if (err)
        {
            LOG_ERROR("Connector::HandleClose fd:%d - SO_ERROR %d %s", m_iFD, err, strerror(err));
        }
    }
    
    if (m_state != kDisconnected)
    {
        DoClose();
    }
    // 清理所有的缓存buf，如果要保证容灾可信通讯，需上层自己实现
    m_SendBuf.Retrieve(m_SendBuf.ReadableBytes());
    m_RecvBuf.Retrieve(m_RecvBuf.ReadableBytes());
    return 0;
}

int TcpClient::_Connected()
{
    int err = GetSocketError();
    if (err)
    {
        LOG_ERROR("Connector::HandleClose fd:%d - SO_ERROR %d %s", m_iFD, err, strerror(err));
        DoClose();
        return -1;
    }
    m_state= kConnected;
    EnableReading();
    //LOG_ERROR("test %d send_buf %d", m_iFD, m_SendBuf.ReadableBytes());
    if (m_SendBuf.ReadableBytes() > 0)
    {
        EnableWriting();
        return DoContineSend();
    }
    else
    {
        //LOG_ERROR("DisableWriting %d", m_iFD);
        DisableWriting();
    }
    
    return 0;
}

int TcpClient::SendMsg( const char * data, size_t len )
{
    LOG_DEBUG("SendMsg fd:%d len:%d m_state:%d", m_iFD, len, m_state);
    if (m_state == kConnected)
    {
        return TcpChannel::DoSend(data, len);
    }
    else if (m_state == kDisconnected)
    {
        if (m_isAutoReconnect)
        {
            LOG_ERROR("SendMsg::kDisconnected - start AutoReconnect");
            TcpChannel::AppendSendBuf(data, len);
            return Connect();
        }

        LOG_ERROR("SendMsg::kDisconnected - but not AutoReconnect");
        return -1;
    }
    else if (m_state == kConnecting)
    {
        TcpChannel::AppendSendBuf(data, len);
        return 0;
    }

    LOG_ERROR("SendMsg::state:%d error", m_state);
    return -1;
}

int TcpClient::OnRecvMsg()
{
    LOG_ERROR("Recv Msg buffer len %d", m_RecvBuf.ReadableBytes());
    return 0;
}

void TcpClient::DoClose()
{
    Close();
    m_state = kDisconnected;
}
