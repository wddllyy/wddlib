#include "epollsvr.h"
#include "connsvr/protocol/protocol_connsvr.pb.h"
#include "connsvr/msghandle/MsgHandle.h"


int ConnClient::OnRecvMsg()
{
	while(true)
	{
		printf("Recv %d bytes Msg %s ", (int)m_RecvBuf.ReadableBytes(), m_RecvBuf.Peek() );
		uint32_t len = m_RecvBuf.ReadableBytes();
		const char* buff = G_ConnSvr.GetMsg(m_RecvBuf.Peek(),len);
	
		if( buff != NULL )
		{
			ConnSvr_Conf::ConnsvrMsg msg;
			if( msg.ParseFromArray(buff,len) )
			{
				MsgHandleMgr::HandleMsg(msg,*this);
			}
		}
		else
		{
			return 0;
		}
		m_RecvBuf.Retrieve(m_RecvBuf.ReadableBytes());
	}
    return 0;
}


int EpollServer::OnMsgRecv(ServerChannel& channel)
{
	ConnectionRecord* rec = SEpollServer::GetInstance()->GetConnectionRecord(channel.m_id);
	if( rec != NULL )
	{
		while(true)
		{
			const char* buf = channel.PeekReadBuf();
			uint32_t len = channel.ReadableBytes();
			if( len == 0 ) return 0;
			uint32_t pos = G_ConnSvr.GetConf()->msglenpos();
			uint32_t typesize = G_ConnSvr.GetConf()->msglensize();
			if( pos >= len )
			{
				LOG_DEBUG("unfini msg pos %u , %u len", pos, len);
				return 0;
			}
			uint32_t msglen = 0;
			switch(typesize)
			{
			case 4:
				msglen =  ntohl(*(uint32_t*)buf);
			case 2:
				msglen = ntohs(*(uint16_t*)buf);
			}

			if( msglen > len )
			{
				LOG_DEBUG("unfini msg msglen %u , %u len", msglen, len);
				return 0;
			}

			ConnSvr_Conf::ConnsvrMsg msgntf;
			msgntf.mutable_head()->set_cmdid(ConnSvr_Conf::connsvr_msg_ntf);
			msgntf.mutable_head()->set_msglen(0);
			msgntf.mutable_head()->set_connid(rec->GetConnid());
			msgntf.mutable_head()->set_port(rec->GetAddr().ToPort());
			msgntf.mutable_head()->set_ip(rec->GetAddr().ToIp());
			msgntf.mutable_msgntf()->set_buff(buf,msglen);

			uint32_t packlen = 0;
			const char* packbuf = G_ConnSvr.Pack(&msgntf,packlen);
			if( packlen > 0 )
			{
				SEpollServer::GetInstance()->SendMsg(rec->GetChannel(),packbuf,packlen);
			}

			LOG_DEBUG("send msg from connid %d to channel %d msglen %u" ,rec->GetConnid(),rec->GetChannel() , msglen);

			channel.RetrieveReadBuf(channel.ReadableBytes());

		}
	}
    return 0;
}
int EpollServer::SendMsg(int channelid , const char* buf,uint32_t len)
{
	if( IsChannelIdValid(channelid) ) 
		return m_cptrvec[channelid]->SendMsg(buf,len);
	return -1;
}

int EpollServer::OnNewChannel(int iFD, InetAddress addr)
{
	if( TcpServer::OnNewChannel(iFD, addr) != 0 )
		return -1;
	//TcpServer::ChannleMap::iterator it = m_ChannelMap.find(iFD);
	//if( it == m_ChannelMap.end() )
	//	return -1;
	int connid = _RecoredNewConn(iFD,addr);
	if( connid < 0 )
	{
		return -1;
	}
	ConnSvr_Conf::ConnsvrMsg msg;
	msg.mutable_head()->set_cmdid(ConnSvr_Conf::connsvr_start_req);
	msg.mutable_head()->set_msglen(0);
	msg.mutable_head()->set_connid(connid);
	msg.mutable_head()->set_port(addr.ToPort());
	msg.mutable_head()->set_ip(addr.ToIp());
	msg.mutable_startreq()->set_currentconn(GetCurrConn());
	msg.mutable_startreq()->set_channelcnt(m_cptrvec.size());
	uint32_t len = 0;
	const char* buf = G_ConnSvr.Pack(&msg,len);
	if( len > 0 )
	{
		m_cptrvec[0]->SendMsg(buf,len);
	}
	else
	{
		_CloseConn(connid);
	}
    printf("OnNewChannel %d from %s count:%d\n", iFD, addr.ToIpPort(), (int)m_ChannelMap.size());
    return 0;
}
int EpollServer::OnCloseChannel( ServerChannel& channel )
{
	ConnectionRecord* rec = SEpollServer::GetInstance()->GetConnectionRecord(channel.m_id);
	if( rec != NULL )
	{
		ConnSvr_Conf::ConnsvrMsg stopmsg;
		stopmsg.mutable_head()->set_cmdid(ConnSvr_Conf::connsvr_stop);
		stopmsg.mutable_head()->set_msglen(0);
		stopmsg.mutable_head()->set_connid(channel.m_id);
		stopmsg.mutable_head()->set_port(rec->GetAddr().ToPort());
		stopmsg.mutable_head()->set_ip(rec->GetAddr().ToIp());
		stopmsg.mutable_stop()->set_timestamp(G_ConnSvr.CurrentTime().tv_sec);
		uint32_t len = 0;
		const char* buf = G_ConnSvr.Pack(&stopmsg,len);
		if( len > 0 )
		{
			channel.SendMsg(buf,len);
		}
	}
	_CloseConn(channel.m_id);
    printf("OnCloseChannel %d from %s count:%d\n", channel.GetFD(), channel.GetPeerAddr().ToIpPort(), (int)m_ChannelMap.size() );
    return TcpServer::OnCloseChannel(channel);
}



