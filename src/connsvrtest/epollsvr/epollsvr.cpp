#include "epollsvr.h"
#include "connsvrtest/protocol/protocol_connsvr.pb.h"
#include "connsvrtest/msghandle/MsgHandle.h"


int EpollServer::OnMsgRecv(ServerChannel& channel)
{
	while(true)
	{
		const char * buf = channel.PeekReadBuf();
		uint32_t len = channel.ReadableBytes();
		const char* buff = G_ConnSvr.GetMsg(buf,len);
		if( buff != NULL )
		{
			ConnSvr_Conf::ConnsvrMsg msg;
			if( msg.ParseFromArray(buff,len) )
			{
				MsgHandleMgr::HandleMsg(msg,channel);
			}
		}
		else
		{
			return 0;
		}
		channel.RetrieveReadBuf(channel.ReadableBytes());
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



