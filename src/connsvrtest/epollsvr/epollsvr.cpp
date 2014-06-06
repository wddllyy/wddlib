#include "epollsvr.h"
#include "connsvrtest/protocol/protocol_connsvr.pb.h"
#include "connsvrtest/msghandle/MsgHandle.h"


int EpollServer::OnMsgRecv(ServerChannel& channel)
{
	while(true)
	{
		int len = channel.ReadableBytes();
		LOG_DEBUG("buflen %u",len);
		if( len == 0 )
			break;
		ConnSvr_Conf::ConnsvrMsg msg;
		const char* buf = channel.PeekReadBuf();
		if( G_ConnSvr.m_parser.UnPack(msg,buf,len) == 0)
		{
			MsgHandleMgr::HandleMsg(msg,channel);
			LOG_DEBUG("retrive len %u",len);
			channel.RetrieveReadBuf(len);
			LOG_DEBUG("left buf len %u",channel.ReadableBytes());
		}
		else
		{
			break;
		}
		
	}
    return 0;
}


int EpollServer::OnNewChannel(int iFD, InetAddress addr)
{
	LOG_DEBUG("new conn accpet %s:%u",addr.ToIp(),addr.ToPort());
	return TcpServer::OnNewChannel(iFD,addr);
}
int EpollServer::OnCloseChannel( ServerChannel& channel )
{
    printf("OnCloseChannel %d from %s count:%d\n", channel.GetFD(), channel.GetPeerAddr().ToIpPort(), (int)m_ChannelMap.size() );
    return TcpServer::OnCloseChannel(channel);
}



