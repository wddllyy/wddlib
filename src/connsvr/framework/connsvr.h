#ifndef __CONNSVR_FRAMEWORK_CONNSVR_H__
#define __CONNSVR_FRAMEWORK_CONNSVR_H__

#include "util/framework/server_app.h"
#include "util/log/logmgr.h"
#include "connsvr.pb.h"
#include "util/pbparser/PbParser.h"
class ConnSvr;

extern ConnSvr G_ConnSvr;

class ConnSvr : public ServerApp
{
public:
    ConnSvr()
        : ServerApp(InetAddress("0.0.0.0", 7777), "connsvr.conf")
    {

    }
    ~ConnSvr()
    {

    }
	EPollPoller& GetPoll() { return m_poll; }
    virtual int OnInit();
    virtual ProcRetMode OnProc();
    virtual int OnTick();
    virtual int OnFini();
    virtual int OnReload();
    virtual ConnSvr_Conf::ConnSvrCfg* GetConf() ;
    virtual int OnCtrlCmd(const std::string& , std::string& );
	ConnSvr_Conf::ConnSvrCfg m_config;
	PbParser m_parser;
};

#endif

