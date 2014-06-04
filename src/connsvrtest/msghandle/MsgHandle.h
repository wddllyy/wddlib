/*******************************************************************************
@ ∞Ê»®À˘”–(C) Tencent 2014
********************************************************************************/
#ifndef _TESTCONNSVR_MSGHANDLE_MSGHANDLE_H__
#define _TESTCONNSVR_MSGHANDLE_MSGHANDLE_H__

#include "connsvrtest/protocol/protocol_connsvr.pb.h"
#include "connsvrtest/epollsvr/epollsvr.h"

typedef int MsgHandle(const ConnSvr_Conf::ConnsvrMsg&, ServerChannel& channel);

class MsgHandleMgr
{
public:
	static int HandleMsg(const ConnSvr_Conf::ConnsvrMsg&, ServerChannel& channel);
	static void Init();
	static MsgHandle* m_handleArr[ConnSvr_Conf::connsvr_cmd_count];
};


#endif //_CONNSVR_MSGHANDLE_MSGHANDLE_H__
