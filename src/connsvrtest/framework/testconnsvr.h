#ifndef __TESTCONNSVR_FRAMEWORK_CONNSVR_H__
#define __TESTCONNSVR_FRAMEWORK_CONNSVR_H__

#include "util/framework/server_app.h"
#include "util/log/logmgr.h"
#include "testconnsvr.pb.h"
#include "util/pbparser/PbParser.h"

class testConnSvr;

extern testConnSvr G_ConnSvr;

class testConnSvr : public ServerApp
{
public:
    testConnSvr()
        : ServerApp(InetAddress("0.0.0.0", 7777), "testconnsvr.conf")
    {

    }
    ~testConnSvr()
    {

    }
	EPollPoller& GetPoll() { return m_poll; }
    virtual int OnInit();
    virtual ProcRetMode OnProc();
    virtual int OnTick();
    virtual int OnFini();
    virtual int OnReload();
    virtual testConnSvr_Conf::testConnSvrCfg* GetConf() ;
    virtual int OnCtrlCmd(const std::string& , std::string& );
	testConnSvr_Conf::testConnSvrCfg m_config;
	PbParser m_parser;
};

#endif

