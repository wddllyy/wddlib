#ifndef __CONNSVR_EPOLLSVR_EPOLLSVR_H__
#define __CONNSVR_EPOLLSVR_EPOLLSVR_H__

#include "util/net/epoll_poller.h"
#include "util/net/tcp_server.h"
#include "util/net/tcp_client.h"
#include "util/log/logmgr.h"
#include "util/singleton/Singleton.h"
#include "connsvr/framework/connsvr.h"
#include <vector>


struct ConnectionRecord
{
	ConnectionRecord()
	{
		memset(this,0,sizeof(ConnectionRecord));
		
	}
	int GetConnid() const
	{
		return m_connid;
	}
	void UpdateConnid()
	{
		m_connid += ((ConnSvr_Conf::ConnSvrCfg*)G_ConnSvr.GetConf())->maxconn();
		if( m_connid < m_idx )
		{
			m_connid = m_idx;
		}
	}
	bool IsTimeout() const
	{
		if( ((ConnSvr_Conf::ConnSvrCfg*)G_ConnSvr.GetConf())->maxidletime() <= 0 ) return false;
		return G_ConnSvr.CurrentTime().tv_sec - m_LastActiveTime >= ((ConnSvr_Conf::ConnSvrCfg*)G_ConnSvr.GetConf())->maxidletime();
	}
	int m_idx;
	int m_connid;
	uint64_t m_LastActiveTime;
};

class ConnClient : public TcpClient
{
public:
    ConnClient(InetAddress addr)
        : TcpClient(G_ConnSvr.GetPoll(), addr)
    {

    }
    virtual int OnRecvMsg();
    
};


class EpollServer : public TcpServer
{
public:
	typedef std::vector<ConnClient*> ClientPtrVec;
	typedef std::vector<ConnectionRecord> CRVector;
	typedef std::vector<int> IDXVec;
    EpollServer( InetAddress addr, bool isReUsePort)
		: TcpServer(G_ConnSvr.GetPoll(), addr, isReUsePort)
    {
		m_vec.resize(G_ConnSvr.GetConf()->maxconn());
		m_idxvec.resize(G_ConnSvr.GetConf()->maxconn());
		for( uint32_t i = 0 ; i < G_ConnSvr.GetConf()->maxconn() ; ++i )
		{
			m_idxvec[i] = i;
			m_vec[i].m_idx = i;
			m_vec[i].m_connid = i;
		}
		m_cptrvec.resize(G_ConnSvr.GetConf()->channel_size() + 1);
		m_cptrvec[0] = new ConnClient(InetAddress(G_ConnSvr.GetConf()->defaultchannel().ip().c_str(),(uint16_t)G_ConnSvr.GetConf()->defaultchannel().port()));

		for( int i = 0 ; i < G_ConnSvr.GetConf()->channel_size() ; ++i )
		{
			m_cptrvec[i+1] = new ConnClient(InetAddress(G_ConnSvr.GetConf()->channel(i).ip().c_str(),(uint16_t)G_ConnSvr.GetConf()->channel(i).port()));
		}
    }
    virtual int OnMsgRecv(ServerChannel& channel);
    virtual int OnNewChannel(int iFD, InetAddress addr);
    virtual int OnCloseChannel( ServerChannel& channel );
	size_t GetCurrConn() const
	{
		return G_ConnSvr.GetConf()->maxconn() - m_idxvec.size();
	}
protected:
	ClientPtrVec m_cptrvec;
	CRVector m_vec;
	IDXVec m_idxvec;
};

typedef SingletonHolder<EpollServer> SEpollServer;



#endif

