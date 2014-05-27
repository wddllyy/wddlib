#ifndef __UTIL_TCP_CHANNEL_H
#define __UTIL_TCP_CHANNEL_H

#include "util/net/tcp_socket.h"
#include "util/net/buffer.h"

class EPollPoller;

class TcpChannel : public TcpSocket
{
public:
    TcpChannel(EPollPoller& poll);
    ~TcpChannel();
public:

    int DoSend(const char * buf, size_t len);
    int DoRecv();
    int DoContineSend();
    virtual int OnRecvMsg() = 0;

    const char * PeekReadBuf(){ return m_RecvBuf.Peek(); }
    size_t ReadableBytes(){ return m_RecvBuf.ReadableBytes(); }
    size_t GetWriteBufBytes(){ return m_SendBuf.ReadableBytes(); }
    size_t RetrieveReadBuf(size_t len){return m_RecvBuf.Retrieve(len);}

    //TODO 这里可以做一个高值预警的callback
    int AppendSendBuf(const char * buf, size_t len);
    
protected:
    Buffer m_RecvBuf;
    Buffer m_SendBuf;


};
#endif
