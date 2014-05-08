#ifndef __UTIL_TCP_SOCKET_H
#define __UTIL_TCP_SOCKET_H

#include <stdint.h>
#include <netinet/in.h>
#include <sys/epoll.h>


class InetAddress
{
public:
    InetAddress();
    explicit InetAddress(uint16_t port);
    InetAddress(const char * ip, uint16_t port);

    InetAddress(const struct sockaddr_in& addr);

    const char * ToIp() const;
    const char * ToIpPort() const;

    const struct sockaddr_in& GetSockAddrInet() const { return addr_; }
    void SetSockAddrInet(const struct sockaddr_in& addr) { addr_ = addr; }

    uint32_t IpNetEndian() const { return addr_.sin_addr.s_addr; }
    uint16_t PortNetEndian() const { return addr_.sin_port; }

private:
    struct sockaddr_in addr_;
};


const int kNoneEvent = 0;
const int kReadEvent = EPOLLIN | EPOLLPRI;
const int kWriteEvent = EPOLLOUT;

class EPollPoller;

class TcpSocket
{
public:
    TcpSocket(EPollPoller& poll);
    ~TcpSocket();
public:
    
    // config setting
    int SetNonBlock();
    int SetReuseAddr(bool on);
    int SetReusePort(bool on);
    int SetTcpNoDelay(bool on);
    int SetKeepAlive(bool on);

    // handle callback
    virtual int HandleCanRecv() = 0;
    virtual int HandleCanSend() = 0;
    virtual int HandleError();

    // only called by read=0 and epoll if ((revents_ & POLLHUP) && !(revents_ & POLLIN))
    virtual int HandleClose() = 0;

    // operator
    int CreateNoBlock();
    int SetFD(int iFD);
    int Close();
    //int ShutdownWrite();
    int GetSocketError();

    // events mask
    void UpdateEvent();
    bool IsNoneEvent() const { return m_iEvents == kNoneEvent; }
    void EnableReading() { m_iEvents |= kReadEvent; UpdateEvent(); }
    void EnableWriting() { m_iEvents |= kWriteEvent; UpdateEvent(); }
    void DisableWriting() { m_iEvents &= ~kWriteEvent; UpdateEvent(); }
    void DisableAll() { m_iEvents = kNoneEvent; UpdateEvent(); }
    bool IsWriting() const { return m_iEvents & kWriteEvent; }
    int GetEpollEvent(){return m_iEvents;}
    int GetFD(){return m_iFD;}

    bool IsPooling();
    void SetPooling(bool on);


    InetAddress GetLocalAddr();
    InetAddress GetPeerAddr();
protected:
    
protected:
    int m_iFD;
    EPollPoller& m_EPoll;
    int m_iEvents;
    bool m_isPooling;


    

};
#endif
