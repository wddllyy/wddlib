#include <sys/socket.h>
#include <unistd.h>
#include <errno.h>

#include "util/net/tcp_channel.h"
#include "util/log/logmgr.h"

static const size_t k_MaxSendBufSize = 50 * 1024 *1024;
static const size_t k_MaxRecvBufSize = 50 * 1024 *1024;

int TcpChannel::DoSend( const char * buf, size_t len )
{
    size_t remaining = len;
    int nwrote = 0;

    if (m_SendBuf.ReadableBytes() == 0)
    {
        nwrote = ::write(m_iFD, buf, len);
        if (nwrote >= 0)
        {
            remaining = len - nwrote;
        }
        else // nwrote < 0
        {
            nwrote = 0;
            int errorcode = errno;
            if (errorcode != EWOULDBLOCK)
            {
                LOG_ERROR("sockets::write ERROR %d", errorcode);
                if (errorcode == EPIPE || errorcode == ECONNRESET) // FIXME: any others? //会在read或者epoll的地方触发close，这里不用处理了
                {
                    LOG_ERROR("sockets::write ERROR EPIPE or ECONNRESET errorcode:%d", errorcode);
                    return -1;
                }
            }
        }
        LOG_TRACE("sockets::write fd:%d len:%d nwrote:%d", m_iFD, len, nwrote);
    }
    
    if (remaining > 0)
    {
        m_SendBuf.ReadableBytes();
        AppendSendBuf(buf + nwrote, remaining);
        if (!IsWriting())
        {
            LOG_TRACE("sockets::async write fd:%d remaining:%d", m_iFD, remaining);
            EnableWriting();
        }
    }
    return 0;
}


int TcpChannel::DoContineSend()
{
    if (IsWriting())
    {
        size_t nwrote = ::write(m_iFD, m_SendBuf.Peek(), m_SendBuf.ReadableBytes());
        if (nwrote > 0)
        {
            m_SendBuf.Retrieve(nwrote);
            if (m_SendBuf.ReadableBytes() == 0)
            {
                DisableWriting();
            }
        }
        else
        {
            LOG_ERROR("TcpConnection::handleWrite");
        }
    }
    else
    {
        LOG_TRACE("Connection fd = %d is down, no more writing", m_iFD);
    }
    return 0;
}

int TcpChannel::DoRecv()
{

    if (m_RecvBuf.ReadableBytes() > k_MaxRecvBufSize)
    {
        m_RecvBuf.Retrieve(m_RecvBuf.ReadableBytes());
    }

    // saved an ioctl()/FIONREAD call to tell how much to read
    char extrabuf[65536];
    struct iovec vec[2];
    const size_t writable = m_RecvBuf.WritableBytes();
    vec[0].iov_base = m_RecvBuf.BeginWrite();
    vec[0].iov_len = writable;
    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof extrabuf;
    // when there is enough space in this buffer, don't read into extrabuf.
    // by doing this, we read 128k-1 bytes at most
    const int iovcnt = (writable < sizeof extrabuf) ? 2 : 1;
    const int n = ::readv(m_iFD, vec, iovcnt);
    if (n == 0)
    {
        HandleClose();
        return 0;
    }
    else if (n < 0)
    {
        int errorcode = errno;
        HandleError();
        LOG_ERROR("sockets::readv ERROR %d", errorcode);
        return -1;
    }
    else
    {
		LOG_TRACE("sockets recv msg len %d", n);
        if (static_cast<size_t>(n) <= writable)
        {
            m_RecvBuf.Writed(n);
        }
        else
        {
			m_RecvBuf.Writed(writable);
            m_RecvBuf.Append(extrabuf, n - writable);
        }
        OnRecvMsg();
    }

    return n;
}

TcpChannel::TcpChannel( EPollPoller& poll )
    :TcpSocket(poll)
{

}

TcpChannel::~TcpChannel()
{

}

int TcpChannel::AppendSendBuf( const char * buf, size_t len )
{
    if (m_SendBuf.ReadableBytes() > k_MaxSendBufSize)
    {
        m_SendBuf.Retrieve(m_SendBuf.ReadableBytes());
    }
    m_SendBuf.Append(buf, len);
    return 0;
}

