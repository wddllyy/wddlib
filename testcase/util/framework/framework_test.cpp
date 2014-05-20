#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string>
#include <sstream>
#include <iostream>
#include "util/framework/server_app.h"
#include "testcase_util.h"
#include "util/log/logmgr.h"
#include "test.pb.h"


class ClientServer : public TcpServer
{
public:
    ClientServer(EPollPoller& poll, InetAddress addr, bool isReUsePort)
        : TcpServer(poll, addr, isReUsePort)
    {
        gettimeofday(&starttime, NULL);
    }
    virtual int OnMsgRecv(ServerChannel& channel)
    {
        LOG_ERROR("Recv %d bytes Msg %s ", (int)channel.ReadableBytes(), channel.PeekReadBuf() );
        channel.SendMsg(channel.PeekReadBuf(), channel.ReadableBytes());


        channel.RetrieveReadBuf(channel.ReadableBytes());

        static int successcount = 0;
        successcount++;
        if (successcount > 100000 )
        {
            struct timeval now ;
            gettimeofday(&now, NULL);

            float diff =   (now.tv_sec-starttime.tv_sec) + ((now.tv_usec - starttime.tv_usec))/1000000.f;
            m_tps = (float)successcount / diff;
            printf("count %d, tps %f\n",successcount, (float)successcount / diff);
            starttime = now;
            successcount = 1;
        }


        return 0;
    }
    virtual int OnNewChannel(int iFD, InetAddress addr)
    {
        LOG_ERROR("OnNewChannel %d from %s count:%d\n", iFD, addr.ToIpPort(), (int)m_ChannelMap.size());
        return TcpServer::OnNewChannel(iFD, addr);
    }
    virtual int OnCloseChannel( ServerChannel& channel )
    {
        LOG_ERROR("OnCloseChannel %d from %s count:%d\n", channel.GetFD(), channel.GetPeerAddr().ToIpPort(), (int)m_ChannelMap.size() );
        return TcpServer::OnCloseChannel(channel);
    }
    int GetTps()
    {
        return m_tps;
    }
    int GetClientCount()
    {
        return m_ChannelMap.size();
    }
    int CloseOne()
    {
        ChannleMap::iterator ite = m_ChannelMap.begin();
        if (ite != m_ChannelMap.end())
        {
            ite->second->Disconnect();
        }
        return 0;
    }
private:
    struct timeval starttime ;
    float m_tps;

};


class TestAppSvr : public ServerApp
{
public:
    TestAppSvr()
        : ServerApp(InetAddress("0.0.0.0", 7777), "testapp.conf")
        , m_client_svr(ServerApp::m_poll, InetAddress("0.0.0.0", 7789), true)
    {

    }
    ~TestAppSvr()
    {

    }
    virtual int OnInit() 
    {
        m_client_svr.Start();
        LOG_ERROR("OnInit");
        return 0;
    }
    virtual ProcRetMode OnProc()
    {
        //LOG_ERROR("OnProc");
        return PROC_RET_SLEEP;
    }
    virtual int OnTick() 
    {
        //LOG_ERROR("OnTick");
        return 0;
    }
    virtual int OnFini() 
    {
        LOG_ERROR("OnTick");
        return 0;
    }
    virtual int OnReload() 
    {
        LOG_ERROR("OnReload");
        LOG_ERROR("config : %s", m_config.DebugString().c_str()); 
        return 0;
    }
    virtual ::google::protobuf::Message* GetConf() 
    {
        return &m_config;
    }
    virtual int OnCtrlCmd(const std::string& instr, std::string& outstr)
    {
        std::stringstream instream;
        std::stringstream outstream;

        instream << instr;
        std::string cmd;

        instream >> cmd;

        if (cmd == "status")
        {
            outstream << "tps: "<< m_client_svr.GetTps() << " clientcount: "<< m_client_svr.GetClientCount() << std::endl;
            outstream << "";
        }
        else if (cmd == "closefd")
        {
            int ret = m_client_svr.CloseOne();
            outstream << "ret: "<< ret << " operato finish "<< std::endl;
        }
        else
        {
            return -1;
        }
        outstream << "\n\0";
        outstr = outstream.str();
        return 0;
    }
private:

    ::controlconfig::Config m_config;
    ClientServer m_client_svr;
};


int util_framework_test(int argc, char* argv[])
{
    GetDefaultLogMgr().AddFileCat(LOGLV_FATAL, LOGLV_FATAL, 20*1024*1024, 5, "framework_test", "log");
    TestAppSvr svr;
    svr.Run();
    return 0;
}

int main(int argc, char* argv[])
{
    util_framework_test(argc, argv);
    return 0;
}