#ifndef __UTIL_SERVER_APP_H
#define __UTIL_SERVER_APP_H

#include <sys/time.h>
#include <string>
#include "util/net//tcp_server.h"


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

    int Run();
    int Init();
    int ControlCmd(const std::string& instr, std::string& outstr);

    template<typename TConf>
    int LoadConf(TConf & conf);
protected:

    int _LoadConf();


    EPollPoller m_poll;
    AppCtrlServer m_ctrl_svr;
    std::string m_conf_filename;
    timeval m_curtime;
    bool m_is_stop;
private:

};

#endif
