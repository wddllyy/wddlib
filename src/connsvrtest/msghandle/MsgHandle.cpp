#include "MsgHandle.h"
#include "util/log/logmgr.h"
#include "connsvrtest/epollsvr/epollsvr.h"

MsgHandle* MsgHandleMgr::m_handleArr[ConnSvr_Conf::connsvr_cmd_count];

int  StartRspHandle(const ConnSvr_Conf::ConnsvrMsg& msg, ServerChannel& channel)
{
	ConnSvr_Conf::ConnsvrMsg startrsp;
	startrsp.mutable_head()->set_cmdid(ConnSvr_Conf::connsvr_start_rsp);
	startrsp.mutable_head()->set_connid(msg.head().connid());
	startrsp.mutable_head()->set_port(msg.head().port());
	startrsp.mutable_head()->set_ip(msg.head().ip());
	startrsp.mutable_startrsp()->set_accept(0);
	startrsp.mutable_startrsp()->set_routechannel(0);

	const char* buf = NULL;
	int len = 0;
	if( G_ConnSvr.m_parser.Pack(startrsp,buf,len) == 0)\
	{
		channel.SendMsg(buf,len);
		LOG_DEBUG("send msg len %d",len);
	}

	return 0;
}

int StopHandle(const ConnSvr_Conf::ConnsvrMsg& , ServerChannel& )
{


	return 0;
}

int RouteHandle(const ConnSvr_Conf::ConnsvrMsg& , ServerChannel& )
{
	return 0;
}

int MsgNtfHandle(const ConnSvr_Conf::ConnsvrMsg& msg, ServerChannel& channel)
{
	ConnSvr_Conf::ConnsvrMsg ntf = msg;

	const char* buf = NULL;
	int len = 0;
	if( G_ConnSvr.m_parser.Pack(ntf,buf,len) == 0 )
	{
		LOG_DEBUG("Send msg to connsvr len %u",len);
		channel.SendMsg(buf,len);
	}

	return 0;
}

void MsgHandleMgr::Init()
{
	memset(m_handleArr,0,sizeof(m_handleArr));

	m_handleArr[ConnSvr_Conf::connsvr_start_req] = &StartRspHandle;
	m_handleArr[ConnSvr_Conf::connsvr_stop] = &StopHandle;
	m_handleArr[ConnSvr_Conf::connsvr_route] = &RouteHandle;
	m_handleArr[ConnSvr_Conf::connsvr_msg_ntf] = &MsgNtfHandle;

}
int MsgHandleMgr::HandleMsg(const ConnSvr_Conf::ConnsvrMsg& msg, ServerChannel& channel)
{
	if( msg.head().cmdid() < ConnSvr_Conf::connsvr_cmd_count )
	{
		if( m_handleArr[msg.head().cmdid()] != NULL )
			return (*m_handleArr[msg.head().cmdid()])(msg,channel);
	}
	LOG_DEBUG("Cannot found handler for %u",msg.head().cmdid());
	return -1;
}

