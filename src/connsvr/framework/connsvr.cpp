#include "connsvr.h"
#include "connsvr/epollsvr/epollsvr.h"
#include "connsvr/msghandle/MsgHandle.h"

ConnSvr G_ConnSvr;

int ConnSvr::OnInit() 
{
	if( GetConf()->msglensize() != 4 &&GetConf()->msglensize() != 2 )
	{
		LOG_ERROR("msglensize can only 2 or 4");
		return -1;
	}
	MsgHandleMgr::Init();
	SEpollServer::SetSingleton(new EpollServer(InetAddress("0.0.0.0", m_config.port()), true));
	SEpollServer::GetInstance()->Start();
    LOG_ERROR("OnInit");
    return 0;
}
ProcRetMode ConnSvr::OnProc()
{
    //LOG_ERROR("OnProc");
    return PROC_RET_SLEEP;
}
int ConnSvr::OnTick() 
{
    //LOG_ERROR("OnTick");
    return 0;
}
int ConnSvr::OnFini() 
{
    LOG_ERROR("OnTick");
    return 0;
}
int ConnSvr::OnReload() 
{
    LOG_ERROR("OnReload");
    LOG_ERROR("config : %s", m_config.DebugString().c_str()); 
    return 0;
}
ConnSvr_Conf::ConnSvrCfg* ConnSvr::GetConf() 
{
    return &m_config;
}
int ConnSvr::OnCtrlCmd(const std::string& , std::string& )
{
    return 0;
}

int main(int argc, char* argv[])
{
    GetDefaultLogMgr().AddFileCat(LOGLV_DEBUG, LOGLV_FATAL, 20*1024*1024, 5, "connsvr", "log");
    G_ConnSvr.Run();
    return 0;

}

