#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <iostream>
#include "util/framework/server_app.h"
#include "google/protobuf/message.h"
#include <google/protobuf/text_format.h>
#include <google/protobuf/io/zero_copy_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>  
#include "util/log/logmgr.h"

#include <algorithm> 
#include <functional> 
#include <cctype>
#include <locale>
#include <arpa/inet.h>

// trim from start
static inline std::string &ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), std::not1(std::ptr_fun<int, int>(std::isspace))));
    return s;
}

// trim from end
static inline std::string &rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), s.end());
    return s;
}

// trim from both ends
static inline std::string &trim(std::string &s) {
    return ltrim(rtrim(s));
}

int AppCtrlServer::OnMsgRecv( ServerChannel& channel )
{
    LOG_INFO("Recv %d bytes Msg %s ", (int)channel.ReadableBytes(), channel.PeekReadBuf() );        // 优化，扫描过的记下来，RetrieveReadBuf清理    //int processcount = 0;    for (size_t i = 0; i < channel.ReadableBytes(); ++i)
    {
        LOG_ERROR("%d", channel.PeekReadBuf()[i]);
    }    while(channel.ReadableBytes() > 0)    {        const char * p_buf = channel.PeekReadBuf();        bool process = false;        if (p_buf[0] == '\r')
        {
            LOG_INFO("RetrieveReadBuf r ");
            channel.RetrieveReadBuf(1);
            process = true;
            continue;
        }        for (size_t i = 0 ; i < channel.ReadableBytes(); ++i)
        {
            if (p_buf[i] == '\n')
            {
                if (i != 0)
                {
                    std::string instr(p_buf, i);
                    std::string outstr;

                    LOG_INFO("Process %d byte msg: %s ", instr.length(), instr.c_str() );

                    m_app.ControlCmd(instr, outstr);
                    
                    LOG_INFO("Send %d byte msg: %s ", outstr.length(), outstr.c_str() );
                    channel.SendMsg(outstr.c_str(), outstr.length());
                }
                LOG_INFO("RetrieveReadBuf %d byte ", i+1);
                channel.RetrieveReadBuf(i+1);
                process = true;
                break;
            }
        }                if (!process)
        {
            break;
        }    }    
    

    return 0;
}

int AppCtrlServer::OnNewChannel( int iFD, InetAddress addr )
{
    LOG_INFO("OnNewChannel %d from %s count:%d\n", iFD, addr.ToIpPort(), (int)m_ChannelMap.size());
    return TcpServer::OnNewChannel(iFD, addr);
}

int AppCtrlServer::OnCloseChannel( ServerChannel& channel )
{
    LOG_INFO("OnCloseChannel %d from %s count:%d\n", channel.GetFD(), channel.GetPeerAddr().ToIpPort(), (int)m_ChannelMap.size() );
    return TcpServer::OnCloseChannel(channel);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

ServerApp::ServerApp( InetAddress controladdr, const char * conf_filename )
    : m_poll()
    , m_ctrl_svr(m_poll, controladdr, *this)
    , m_conf_filename(conf_filename)
    , m_is_stop(false)
{
	
}



int ServerApp::_LoadConf()
{
    int infd = open(m_conf_filename.c_str(), O_RDONLY);
    google::protobuf::io::FileInputStream fileInput(infd);  
    fileInput.SetCloseOnDelete( true ); 
    bool ret = google::protobuf::TextFormat::Parse(&fileInput, GetConf());
    if (!ret)
    {
        LOG_ERROR("google::protobuf::TextFormat::Parse error ");
    }
    return 0;
}


int ServerApp::Init()
{
    m_poll.InitEpoll();
    m_ctrl_svr.Start();

    _LoadConf();

    return OnInit();


}
const char* ServerApp::GetMsg(const char* buff,uint32_t& len)
{
	uint32_t msglen = *(uint32_t*)buff;
	msglen = ntohl(msglen);
	if( len < msglen )
		return NULL;
	len = msglen;
	return buff;
}

const char* ServerApp::Pack(::google::protobuf::Message* msg, uint32_t& len)
{
	len = 0;
	if( msg->SerializeToArray(m_Buff,_Max_Msg_Len) )
	{
		len = msg->ByteSize();
		uint32_t nlen = htonl(len);
		memcpy(m_Buff,&nlen,sizeof(uint32_t));
		return m_Buff;
	}
	return NULL;
}
int ServerApp::Run()
{
    int ret = Init();
    if (ret < 0)
    {
        return -1;
    }
    timeval ticktime;
    gettimeofday(&ticktime, NULL);
    int idlesleepcount = 0;
    for(; !m_is_stop; )
    {
        m_poll.Poll(0);
        ProcRetMode ret = OnProc();
        if (ret == PROC_RET_SLEEP)
        {
            ++idlesleepcount;
        }
        else
        {
            idlesleepcount = 0;
        }
        gettimeofday(&m_curtime, NULL);


        float fcost = (m_curtime.tv_sec - ticktime.tv_sec)*1000 + (m_curtime.tv_usec - ticktime.tv_usec)/1000;

        if (idlesleepcount > 10)
        {
            int alignms = (100 - fcost);
            if (alignms > 0)
            {
                idlesleepcount = 0;
                usleep(alignms*1000);
            }
            
        }

        
        if (fcost >= 100)
        {
            ticktime = m_curtime;
            OnTick();
        }
    }

    OnFini();
    return 0;
}

int ServerApp::ControlCmd( const std::string& instr, std::string& outstr )
{
    std::stringstream instream;
    std::stringstream outstream;

    instream << instr;
    std::string cmd;

    instream >> cmd;
    // TODO : cmd map
    if (cmd == "reload")
    {
        _LoadConf();
        OnReload();
        outstream << "Reload success";
        outstream << "\n\0";
        outstr = outstream.str();
    }
    else if (cmd == "stop")
    {
        m_is_stop = true;
        outstream << "Done";
        outstream << "\n\0";
        outstr = outstream.str();
    }
    else
    {
        int ret = OnCtrlCmd(instr, outstr);
        if (ret < 0)
        {
            LOG_ERROR("unsupport cmd %s", instr.c_str());
            for (size_t i = 0; i < instr.length(); ++i)
            {
                LOG_ERROR("%d", instr[i]);
            }
            outstream << "unsupport cmd " << instr;
            outstream << "\n\0";
            outstr = outstream.str();
        }
    }

    return 0;
}

ServerApp::~ServerApp()
{

}

int ServerApp::OnCtrlCmd( const std::string& instr, std::string& outstr )
{
    return -1;
}
