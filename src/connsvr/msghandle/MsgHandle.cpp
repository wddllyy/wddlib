#include "MsgHandle.h"
#include "util/log/logmgr.h"
#include "connsvr/epollsvr/epollsvr.h"

MsgHandle* MsgHandleMgr::m_handleArr[ConnSvr_Conf::connsvr_cmd_count];

int  StartRspHandle(const ConnSvr_Conf::ConnsvrMsg& msg, ConnClient& )
{
	ConnectionRecord* rec = SEpollServer::GetInstance()->GetConnectionRecord(msg.head().connid());
	if( rec == NULL ) return -1;
	if( msg.startrsp().accept() != 0 )
	{
		SEpollServer::GetInstance()->CloseConnid(msg.head().connid());
		return 0;
	}
	if( SEpollServer::GetInstance()->IsChannelIdValid(msg.startrsp().routechannel()))
	{
		rec->SetChannelid(msg.startrsp().routechannel());
	}
	else
	{
		SEpollServer::GetInstance()->CloseConnid(msg.head().connid());
		return 0;
	}
	return 0;
}

int StopHandle(const ConnSvr_Conf::ConnsvrMsg& msg, ConnClient& channel)
{
	ConnectionRecord* rec = SEpollServer::GetInstance()->GetConnectionRecord(msg.head().connid());
	if( rec == NULL ) return -1;

	ConnSvr_Conf::ConnsvrMsg stopmsg;
	stopmsg.mutable_head()->set_cmdid(ConnSvr_Conf::connsvr_stop);
	stopmsg.mutable_head()->set_msglen(0);
	stopmsg.mutable_head()->set_connid(msg.head().connid());
	stopmsg.mutable_head()->set_port(rec->GetAddr().ToPort());
	stopmsg.mutable_head()->set_ip(rec->GetAddr().ToIp());
	stopmsg.mutable_stop()->set_timestamp(G_ConnSvr.CurrentTime().tv_sec);
	SEpollServer::GetInstance()->CloseConnid(msg.head().connid());
	uint32_t len = 0;
	const char* buf = G_ConnSvr.Pack(&stopmsg,len);
	if( len > 0 )
	{
		channel.SendMsg(buf,len);
	}

	return 0;
}

int RouteHandle(const ConnSvr_Conf::ConnsvrMsg& msg, ConnClient& channel)
{
	ConnectionRecord* rec = SEpollServer::GetInstance()->GetConnectionRecord(msg.head().connid());
	if( rec == NULL ) return -1;

	ConnSvr_Conf::ConnsvrMsg routemsg;
	routemsg.mutable_head()->set_cmdid(ConnSvr_Conf::connsvr_route);
	routemsg.mutable_head()->set_msglen(0);
	routemsg.mutable_head()->set_connid(msg.head().connid());
	routemsg.mutable_head()->set_port(rec->GetAddr().ToPort());
	routemsg.mutable_head()->set_ip(rec->GetAddr().ToIp());
	routemsg.mutable_route()->set_routechannel(rec->GetChannel());

	if( SEpollServer::GetInstance()->IsChannelIdValid(msg.startrsp().routechannel()))
	{
		rec->SetChannelid(msg.startrsp().routechannel());
		routemsg.mutable_route()->set_routechannel(rec->GetChannel());
	}
	uint32_t len = 0;
	const char* buf = G_ConnSvr.Pack(&routemsg,len);
	if( len > 0 )
	{
		channel.SendMsg(buf,len);
	}
	return 0;
}

int MsgNtfHandle(const ConnSvr_Conf::ConnsvrMsg& msg, ConnClient& channel)
{
	ConnectionRecord* rec = SEpollServer::GetInstance()->GetConnectionRecord(msg.head().connid());
	if( rec == NULL )
	{
		ConnSvr_Conf::ConnsvrMsg stopmsg;
		stopmsg.mutable_head()->set_cmdid(ConnSvr_Conf::connsvr_stop);
		stopmsg.mutable_head()->set_msglen(0);
		stopmsg.mutable_head()->set_connid(msg.head().connid());
		stopmsg.mutable_head()->set_port(rec->GetAddr().ToPort());
		stopmsg.mutable_head()->set_ip(rec->GetAddr().ToIp());
		stopmsg.mutable_stop()->set_timestamp(G_ConnSvr.CurrentTime().tv_sec);
		uint32_t len = 0;
		const char* buf = G_ConnSvr.Pack(&stopmsg,len);
		if( len > 0 )
		{
			channel.SendMsg(buf,len);
		}
		return -1;
	}
	ServerChannel* sc = SEpollServer::GetInstance()->GetSvrChannel(msg.head().connid());
	if( sc == NULL )
	{
		ConnSvr_Conf::ConnsvrMsg stopmsg;
		stopmsg.mutable_head()->set_cmdid(ConnSvr_Conf::connsvr_stop);
		stopmsg.mutable_head()->set_msglen(0);
		stopmsg.mutable_head()->set_connid(msg.head().connid());
		stopmsg.mutable_head()->set_port(rec->GetAddr().ToPort());
		stopmsg.mutable_head()->set_ip(rec->GetAddr().ToIp());
		stopmsg.mutable_stop()->set_timestamp(G_ConnSvr.CurrentTime().tv_sec);
		SEpollServer::GetInstance()->CloseConnid(msg.head().connid());
		uint32_t len = 0;
		const char* buf = G_ConnSvr.Pack(&stopmsg,len);
		if( len > 0 )
		{
			channel.SendMsg(buf,len);
		}
		SEpollServer::GetInstance()->CloseConnid(msg.head().connid());
		return -1;
	}

	sc->SendMsg(msg.msgntf().buff().c_str(),msg.msgntf().buff().size());
	
	LOG_DEBUG("Send msg to client connid %d , len %u",msg.head().connid(),msg.msgntf().buff().size());

	return 0;
}

void MsgHandleMgr::Init()
{
	memset(m_handleArr,0,sizeof(m_handleArr));

	m_handleArr[ConnSvr_Conf::connsvr_start_rsp] = &StartRspHandle;
	m_handleArr[ConnSvr_Conf::connsvr_stop] = &StopHandle;
	m_handleArr[ConnSvr_Conf::connsvr_route] = &RouteHandle;
	m_handleArr[ConnSvr_Conf::connsvr_msg_ntf] = &MsgNtfHandle;

}
int MsgHandleMgr::HandleMsg(const ConnSvr_Conf::ConnsvrMsg& msg, ConnClient& channel)
{
	if( msg.head().cmdid() < ConnSvr_Conf::connsvr_cmd_count )
	{
		if( m_handleArr[msg.head().cmdid()] != NULL )
			return (*m_handleArr[msg.head().cmdid()])(msg,channel);
	}
	LOG_DEBUG("Cannot found handler for %u",msg.head().cmdid());
	return -1;
}

