/*******************************************************************************
@ ∞Ê»®À˘”–(C) Tencent 2014
********************************************************************************/
#ifndef _CONNSVR_MSGHANDLE_MSGHANDLE_H__
#define _CONNSVR_MSGHANDLE_MSGHANDLE_H__

#include "connsvr/protocol/protocol_connsvr.pb.h"
#include "connsvr/epollsvr/epollsvr.h"

typedef int MsgHandle(const ConnSvr_Conf::ConnsvrMsg&, ConnClient& channel);

class MsgHandleMgr
{
public:
	static int HandleMsg(const ConnSvr_Conf::ConnsvrMsg&, ConnClient& channel);
	static void Init();
	static MsgHandle* m_handleArr[ConnSvr_Conf::connsvr_cmd_count];
};


#endif //_CONNSVR_MSGHANDLE_MSGHANDLE_H__
