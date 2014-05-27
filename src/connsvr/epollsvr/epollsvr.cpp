#include "epollsvr.h"
#include "connsvr/protocol/protocol_connsvr.pb.h"

int ConnClient::OnRecvMsg()
{
    printf("Recv %d bytes Msg %s ", (int)m_RecvBuf.ReadableBytes(), m_RecvBuf.Peek() );
    m_RecvBuf.Retrieve(m_RecvBuf.ReadableBytes());

        
    return 0;
}


int EpollServer::OnMsgRecv(ServerChannel& channel)
{


    //printf("Recv %d bytes Msg %s ", (int)channel.ReadableBytes(), channel.PeekReadBuf() );

  /*  channel.SendMsg(channel.PeekReadBuf(), channel.ReadableBytes());
    channel.RetrieveReadBuf(channel.ReadableBytes());
    static int successcount = 0;
    successcount++;
    if (successcount > 100000 )
    {
        struct timeval now ;
        gettimeofday(&now, NULL);

        float diff =   (now.tv_sec-starttime.tv_sec) + ((now.tv_usec - starttime.tv_usec))/1000000.f;
        printf("count %d, tps %f\n",successcount, (float)successcount / diff);
        starttime = now;
        successcount = 1;
    }*/

    return 0;
}
int EpollServer::OnNewChannel(int iFD, InetAddress addr)
{
	if( m_idxvec.empty() )
	{
		m_ChannelMap[iFD]->Close();
	}
	int connid = *m_idxvec.rbegin();
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
    printf("OnNewChannel %d from %s count:%d\n", iFD, addr.ToIpPort(), (int)m_ChannelMap.size());
    return TcpServer::OnNewChannel(iFD, addr);
}
int EpollServer::OnCloseChannel( ServerChannel& channel )
{
    printf("OnCloseChannel %d from %s count:%d\n", channel.GetFD(), channel.GetPeerAddr().ToIpPort(), (int)m_ChannelMap.size() );
    return TcpServer::OnCloseChannel(channel);
}



