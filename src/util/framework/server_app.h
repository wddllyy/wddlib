#ifndef __UTIL_SERVER_APP_H
#define __UTIL_SERVER_APP_H

#include <sys/time.h>
#include <string>
#include "util/net/tcp_server.h"


enum ProcRetMode
{
    PROC_RET_SLEEP = 0,
    PROC_RET_NO_SLEEP = 1,
};

namespace google 
{
    namespace protobuf 
    {
        class Message;
    }
}

#define _Max_Msg_Len 1024*1024

class ServerApp;
class AppCtrlServer : public TcpServer
{
public:
    AppCtrlServer(EPollPoller& poll, InetAddress addr, ServerApp & app)
        : TcpServer(poll, addr, true)
        , m_app(app)
    {

    }
    virtual int OnMsgRecv(ServerChannel& channel);
    virtual int OnNewChannel(int iFD, InetAddress addr);
    virtual int OnCloseChannel( ServerChannel& channel );
private:
    ServerApp& m_app;
};



class ServerApp
{
public:
    ServerApp(InetAddress controladdr, const char * conf_filename);
    virtual ~ServerApp();
    
    virtual int OnInit() = 0;
    virtual ProcRetMode OnProc() = 0;
    virtual int OnTick() = 0;
    virtual int OnFini() = 0;
    virtual int OnReload() = 0;
    virtual ::google::protobuf::Message* GetConf() = 0;
    virtual int OnCtrlCmd(const std::string& instr, std::string& outstr);
	timeval CurrentTime() const { return m_curtime; } 
    int Run();
    int Init();
    int ControlCmd(const std::string& instr, std::string& outstr);
	const char* Pack(::google::protobuf::Message* , uint32_t& len);
	const char* GetMsg(const char* ,uint32_t& len);
    template<typename TConf>
    int LoadConf(TConf & conf);
protected:

    int _LoadConf();


    EPollPoller m_poll;
    AppCtrlServer m_ctrl_svr;
    std::string m_conf_filename;
    timeval m_curtime;
    bool m_is_stop;

	char m_Buff[_Max_Msg_Len];

private:

};

#endif
