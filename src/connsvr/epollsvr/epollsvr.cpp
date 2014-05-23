#include "epollsvr.h"

EPollPoller __unvisiable_poll;

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
    printf("OnNewChannel %d from %s count:%d\n", iFD, addr.ToIpPort(), (int)m_ChannelMap.size());
    return TcpServer::OnNewChannel(iFD, addr);
}
int EpollServer::OnCloseChannel( ServerChannel& channel )
{
    printf("OnCloseChannel %d from %s count:%d\n", channel.GetFD(), channel.GetPeerAddr().ToIpPort(), (int)m_ChannelMap.size() );
    return TcpServer::OnCloseChannel(channel);
}


