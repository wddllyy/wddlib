#ifndef __CONNSVR_FRAMEWORK_CONNSVR_H__
#define __CONNSVR_FRAMEWORK_CONNSVR_H__

#include "util/framework/server_app.h"
#include "util/log/logmgr.h"
#include "connsvr.pb.h"

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
    virtual int OnInit();
    virtual ProcRetMode OnProc();
    virtual int OnTick();
    virtual int OnFini();
    virtual int OnReload();
    virtual ::google::protobuf::Message* GetConf() ;
    virtual int OnCtrlCmd(const std::string& , std::string& );
	ConnSvrCfg m_config;
};

#endif

