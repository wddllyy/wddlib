#include "testconnsvr.h"
#include "connsvrtest/epollsvr/epollsvr.h"
#include "connsvrtest/msghandle/MsgHandle.h"

testConnSvr G_ConnSvr;

int testConnSvr::OnInit() 
{
	MsgHandleMgr::Init();
	SEpollServer::SetSingleton(new EpollServer(InetAddress("0.0.0.0", m_config.port()), true));
	SEpollServer::GetInstance()->Start();
    LOG_ERROR("OnInit");
    return 0;
}
ProcRetMode testConnSvr::OnProc()
{
    //LOG_ERROR("OnProc");
    return PROC_RET_SLEEP;
}
int testConnSvr::OnTick() 
{
    //LOG_ERROR("OnTick");
    return 0;
}
int testConnSvr::OnFini() 
{
    LOG_ERROR("OnTick");
    return 0;
}
int testConnSvr::OnReload() 
{
    LOG_ERROR("OnReload");
    LOG_ERROR("config : %s", m_config.DebugString().c_str()); 
    return 0;
}
testConnSvr_Conf::testConnSvrCfg* testConnSvr::GetConf() 
{
    return &m_config;
}
int testConnSvr::OnCtrlCmd(const std::string& , std::string& )
{
    return 0;
}

int main(int argc, char* argv[])
{
    GetDefaultLogMgr().AddFileCat(LOGLV_TRACE, LOGLV_FATAL, 20*1024*1024, 5, "connsvrtest", "log");
    G_ConnSvr.Run();
    return 0;

}

