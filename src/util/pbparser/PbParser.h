/*******************************************************************************
@ ∞Ê»®À˘”–(C) Tencent 2014
********************************************************************************/
#ifndef __WDDLIB_UTIL_PBPARSER_H__
#define __WDDLIB_UTIL_PBPARSER_H__

#include "google/protobuf/message.h"
#include <vector>


class PbParser
{
public:
	PbParser();
	~PbParser();
	int Init(int buffsize = 4096);
	int Pack(google::protobuf::Message& msg,const char*& ,int& len);
	int UnPack(google::protobuf::Message& msg , const char* buff,int& len);
protected:
	std::vector<char> m_buffer;
};

#endif //__WDDLIB_UTIL_PBPARSER_H__
