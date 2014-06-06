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
	friend class EpollServer;
	ConnectionRecord()
	{
		m_channelid = 0;
		m_iFD = 0;
		memset(&m_addr,0,sizeof(m_addr));
		_UpdateConnid();
	}
	void Attatch(int fd,const InetAddress& addr)
	{
		m_addr = addr;
		m_iFD = fd;
	}
	bool IsValid() const
	{
		return m_iFD > 0;
	}
	void Detatch()
	{
		m_channelid = 0;
		m_iFD = 0;
		memset(&m_addr,0,sizeof(m_addr));
		_UpdateConnid();
	}
	int GetConnid() const
	{
		return m_connid;
	}
	void UpdateActiveTime()
	{
		m_LastActiveTime = G_ConnSvr.CurrentTime().tv_sec;
	}
	bool IsTimeout() const
	{
		if( ((ConnSvr_Conf::ConnSvrCfg*)G_ConnSvr.GetConf())->maxidletime() <= 0 ) return false;
		return G_ConnSvr.CurrentTime().tv_sec - m_LastActiveTime >= ((ConnSvr_Conf::ConnSvrCfg*)G_ConnSvr.GetConf())->maxidletime();
	}
	void SetChannelid(int id)
	{
		m_channelid = id;
	}
	int GetFD() const { return m_iFD; }
	InetAddress& GetAddr() { return m_addr; }
	int GetChannel() const { return m_channelid; } 
	int GetIdx() const { return m_idx; }
private:
	void _UpdateConnid()
	{
		m_connid += ((ConnSvr_Conf::ConnSvrCfg*)G_ConnSvr.GetConf())->maxconn();
		if( m_connid < m_idx )
		{
			m_connid = m_idx;
		}
	}
	int m_idx;
	int m_connid;
	int m_iFD;
	int m_channelid;
	InetAddress m_addr;
	uint64_t m_LastActiveTime;
};

class ConnClient : public TcpClient
{
public:
    ConnClient(InetAddress addr , int channelid)
        : TcpClient(G_ConnSvr.GetPoll(), addr)
		, m_channid(channelid)
    {
    }
	int GetChannId() const { return m_channid; }
    virtual int OnRecvMsg();
	int SendMsg(google::protobuf::Message& );
protected:
	int m_channid;
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
		m_cptrvec[0] = new ConnClient(InetAddress(G_ConnSvr.GetConf()->defaultchannel().ip().c_str(),(uint16_t)G_ConnSvr.GetConf()->defaultchannel().port()),0);
		m_cptrvec[0]->Connect();
		for( int i = 0 ; i < G_ConnSvr.GetConf()->channel_size() ; ++i )
		{
			m_cptrvec[i+1] = new ConnClient(InetAddress(G_ConnSvr.GetConf()->channel(i).ip().c_str(),(uint16_t)G_ConnSvr.GetConf()->channel(i).port()),i+1);
			m_cptrvec[i+1]->Connect();
		}
    }
	bool IsChannelIdValid(int id) const
	{
		return id >= 0 && id < (int)m_cptrvec.size();
	}
    virtual int OnMsgRecv(ServerChannel& channel);
    virtual int OnNewChannel(int iFD, InetAddress addr);
    virtual int OnCloseChannel( ServerChannel& channel );
	int GetFreeIdx()
	{
		if( m_idxvec.empty() ) return -1;
		int connid = *m_idxvec.rbegin();
		m_idxvec.pop_back();
		return connid;
	}
	void ReturnIdx(int idx)
	{
		if( idx >= 0 && idx < (int)G_ConnSvr.GetConf()->maxconn() )
			m_idxvec.push_back(idx);
	}
	size_t GetCurrConn() const
	{
		return G_ConnSvr.GetConf()->maxconn() - m_idxvec.size();
	}
	ConnectionRecord* GetConnectionRecord( int connid ) 
	{
		if( connid >= 0 )
		{
			ConnectionRecord* rec = &m_vec[connid%m_vec.size()];
			if(  rec->IsValid() && rec->GetConnid() == connid )
				return rec;
		}
		return NULL;
	}
	ConnectionRecord* GetConnectionRecordByIdx( int idx ) 
	{
		if( idx >= 0 )
		{
			ConnectionRecord* rec = &m_vec[idx%m_vec.size()];
			if( rec->GetIdx() == idx )
				return rec;
		}
		return NULL;
	}
	void CloseConnid(int connid)
	{
		ConnectionRecord*  rec = GetConnectionRecord(connid);
		if( rec != NULL && rec->IsValid() )
		{
			TcpServer::ChannleMap::iterator it = m_ChannelMap.find(rec->m_iFD);
			if( it != m_ChannelMap.end() )
			{
				it->second->Disconnect();
			}
		}
		_CloseConn(connid);
	}
	void CloseChannel(ServerChannel& channel)
	{
		channel.Disconnect();
		_CloseConn(channel.m_id);
	}

	ServerChannel* GetSvrChannel(int connid)
	{
		ConnectionRecord*  rec = GetConnectionRecord(connid);
		if( rec != NULL )
		{
			TcpServer::ChannleMap::iterator it = m_ChannelMap.find(rec->m_iFD);
			if( it != m_ChannelMap.end() )
			{
				return it->second;
			}
		}
		return NULL;
	}
	int SendMsg(int channelid , google::protobuf::Message& );
protected:
	int _RecoredNewConn(int iFD, const InetAddress& addr)
	{
		int idx = GetFreeIdx();
		ConnectionRecord* connred = GetConnectionRecordByIdx(idx);
		if( connred == NULL || connred->IsValid() )
		{
			_CloseConn(idx);
			return -1;
		}
		connred->Attatch(iFD,addr);
		return connred->GetConnid();
	}
	
	void _CloseConn(int connid)
	{
		ConnectionRecord* connred = GetConnectionRecord(connid);
		if( connred != NULL && connred->IsValid() )
		{
			ReturnIdx(connred->m_idx);
			connred->Detatch();
		}
	}
	ClientPtrVec m_cptrvec;
	CRVector m_vec;
	IDXVec m_idxvec;
};

typedef SingletonHolder<EpollServer> SEpollServer;



#endif

