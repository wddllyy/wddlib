#ifndef __CONNSVR_EPOLLSVR_EPOLLSVR_H__
#define __CONNSVR_EPOLLSVR_EPOLLSVR_H__

#include "util/net/epoll_poller.h"
#include "util/net/tcp_server.h"
#include "util/log/logmgr.h"
#include "util/singleton/Singleton.h"
#include "connsvr/framework/connsvr.h"
#include <vector>

extern EPollPoller __unvisiable_poll;

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

class EpollServer : public TcpServer
{
public:
	typedef std::vector<ConnectionRecord> CRVector;
	typedef std::vector<int> IDXVec;
    EpollServer( InetAddress addr, bool isReUsePort)
        : TcpServer(__unvisiable_poll, addr, isReUsePort)
    {
		m_vec.resize(G_ConnSvr.GetConf()->maxconn());
		m_idxvec.resize(G_ConnSvr.GetConf()->maxconn());
		for( uint32_t i = 0 ; i < G_ConnSvr.GetConf()->maxconn() ; ++i )
		{
			m_idxvec[i] = i;
			m_vec[i].m_idx = i;
			m_vec[i].m_connid = i;
		}
    }
	int Process();
    virtual int OnMsgRecv(ServerChannel& channel);
    virtual int OnNewChannel(int iFD, InetAddress addr);
    virtual int OnCloseChannel( ServerChannel& channel );
protected:
	CRVector m_vec;
	IDXVec m_idxvec;
};

typedef SingletonHolder<EpollServer> SEpollServer;

#endif

