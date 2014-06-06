#include "PbParser.h"
#include "util/log/logmgr.h"
#include "UtilProtocol.pb.h"
#include <arpa/inet.h>

PbParser::PbParser( )
{
	Init();
}
PbParser::~PbParser()
{

}
int PbParser::Init( int buffsize)
{
	m_buffer.reserve(buffsize);
	return 0;
}
int PbParser::Pack(google::protobuf::Message& msg,const char*& buff,int& len)
{
	len = msg.ByteSize();
	LOG_TRACE("try to pack msg %s, len %u", msg.GetTypeName().c_str(), len);
	m_buffer.resize(len + sizeof(uint32_t));
	char* outbuffer = &m_buffer[0];
	char* packbuffer = outbuffer;
	if( msg.GetDescriptor()->options().GetExtension(util::pbparser::PacketOptions).protocolpacket() == util::pbparser::protocol_yes )
	{
		len += sizeof(uint32_t);
		*(uint32_t*)outbuffer = htonl(len);
		packbuffer += sizeof(uint32_t);
		LOG_TRACE("msg is protocol packet real pack len %u", len);
	}
	if( msg.SerializeToArray(packbuffer,msg.ByteSize()) )
	{
		LOG_DEBUG("pack msg:\n %s",msg.DebugString().c_str());
		buff = outbuffer;
		return 0;
	}
	LOG_ERROR("serialize to array fail");
	len = 0;
	return -1;
}
int PbParser::UnPack(google::protobuf::Message& msg , const char* buff,int& len)
{
	uint32_t reallen = len;
	LOG_TRACE("msg buffer len %u",reallen);
	const char* packetbuf = buff;
	if( msg.GetDescriptor()->options().GetExtension(util::pbparser::PacketOptions).protocolpacket() == util::pbparser::protocol_yes )
	{
		reallen = ntohl(*(uint32_t*)buff) - sizeof(uint32_t);
		packetbuf = buff + sizeof(uint32_t);
		LOG_TRACE("msg is protocol packet real pack len %u",reallen);
		if( reallen > len - sizeof(uint32_t) )
		{
			LOG_TRACE("uncomplete buff");
			return -1;
		}
	}
	if( msg.ParseFromArray(packetbuf , reallen) )
	{
		LOG_DEBUG("unpack msg:\n %s",msg.DebugString().c_str());
		len = reallen + sizeof(uint32_t);
		return 0;
	}
	LOG_ERROR("msg unpack fail");
	return -1;
}


