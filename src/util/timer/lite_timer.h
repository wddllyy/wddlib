#ifndef _LITE_TIMER_
#define _LITE_TIMER_

#include <algorithm>
#include <vector>
#include <iterator>
#include <time.h>
#include <stdio.h>
#include "util/container/obj_pool.h"



class ILiteTimerMgr
{
public:

    virtual int GetTimerIdxByID(int id) = 0;
    virtual int SetTimerIdxById(int id, int idx) = 0;
    virtual int StartTimer(time_t expireTime) = 0;
    virtual int StopTimer(int id) = 0;
    virtual int TickOneExpireTimer(time_t now) = 0;
    virtual ~ILiteTimerMgr(){;}
};



struct LiteTimer 
{
    LiteTimer(int id, int idx, time_t expiretime, ILiteTimerMgr & mgr)
        :m_id(id)
        ,m_TimerIdx(idx)
        ,m_expireTime(expiretime)
        ,m_Mgr(mgr)
    {

    }

    bool operator < (const LiteTimer& elem1)
    {
        return m_expireTime > elem1.m_expireTime;
    }

    LiteTimer& operator = ( const LiteTimer& elem )
    {
        m_id = elem.m_id;
        m_expireTime = elem.m_expireTime;

        m_Mgr.SetTimerIdxById(m_id, m_TimerIdx);
        return *this;
    }
    int m_id;
    int m_TimerIdx;
    time_t m_expireTime;
    ILiteTimerMgr & m_Mgr;
};

class IAmTimerMgr : ILiteTimerMgr
{
public:
    IAmTimerMgr();
    ~IAmTimerMgr();
    int Reset();
    int UpdateTimer(int id, time_t expireTime);

    int _StartTimer(int id, time_t expireTime);
    int _StopTimer(int id);
    int _TickOneExpireTimer(time_t now);
protected:
    std::vector<LiteTimer> m_TimerHeap;
};

template<class TTimerBuf>
class LiteTimerMgr : IAmTimerMgr
{
    struct TTimerArg
    {
        int m_iTimerIdx;
        TTimerBuf m_Buf;
    };
public:
    LiteTimerMgr();
    ~LiteTimerMgr();
    int InitTimerMgr(int iMaxCount);
    int Destory();

    int UpdateTimer(int id, time_t expireTime);

    // no use
    int StartTimer(time_t expireTime);

    int StartTimer(time_t expireTime, const TTimerBuf& buf);
    int StopTimer(int id);

    // no use
    int TickOneExpireTimer(time_t now);
    int TickOneExpireTimer(time_t now, TTimerBuf & arg);

    int GetTimerIdxByID(int id);
    int SetTimerIdxById(int id, int idx);

    int GetActiveCount();
private:
    TObjPoolPtr m_pTimerArgPool;
};

template<class TTimerBuf>
int LiteTimerMgr<TTimerBuf>::GetActiveCount()
{
    int iCount1 = objpool_get_used_items(m_pTimerArgPool);
    int iCount2 = m_TimerHeap.size();
    if (iCount1 != iCount2)
    {
        printf("%d-%d \n",iCount1, iCount2);
        return -1;
    }
    return iCount2;
}

template<class TTimerBuf>
LiteTimerMgr<TTimerBuf>::LiteTimerMgr()
    :m_pTimerArgPool(NULL)
{

}

template<class TTimerBuf>
LiteTimerMgr<TTimerBuf>::~LiteTimerMgr()
{

}



template<class TTimerBuf>
int LiteTimerMgr<TTimerBuf>::SetTimerIdxById( int id, int idx )
{
    if (m_pTimerArgPool == NULL)
    {
        return -1;
    }

    TTimerArg* pTimerArg = (TTimerArg*)objpool_get(m_pTimerArgPool, id);
    if (pTimerArg == NULL)
    {
        return -2;
    }
    pTimerArg->m_iTimerIdx = idx;

    return 0;
}

template<class TTimerBuf>
int LiteTimerMgr<TTimerBuf>::GetTimerIdxByID( int id )
{
    if (m_pTimerArgPool == NULL)
    {
        return -1;
    }

    TTimerArg* pTimerArg = (TTimerArg*)objpool_get(m_pTimerArgPool, id);
    if (pTimerArg == NULL)
    {
        return -2;
    }
    return pTimerArg->m_iTimerIdx;
}

template<class TTimerBuf>
int LiteTimerMgr<TTimerBuf>::InitTimerMgr( int iMaxCount )
{
    if (m_pTimerArgPool != NULL)
    {
        return -1;
    }
    int iRet = objpool_init(&m_pTimerArgPool, iMaxCount, sizeof(TTimerArg));
    if (iRet < 0)
    {
        return -2;
    }
    Reset();
    return 0;
}

template<class TTimerBuf>
int LiteTimerMgr<TTimerBuf>::Destory()
{
    if (m_pTimerArgPool != NULL)
    {
        objpool_destroy(&m_pTimerArgPool);
    }
    m_pTimerArgPool = NULL;

    Reset();
    return 0;
}

template<class TTimerBuf>
int LiteTimerMgr<TTimerBuf>::StartTimer( time_t expireTime )
{
    TTimerBuf buf;
    return StartTimer(expireTime, buf);
}

template<class TTimerBuf>
int LiteTimerMgr<TTimerBuf>::StartTimer( time_t expireTime, const TTimerBuf& buf )
{
    if (m_pTimerArgPool == NULL || expireTime == 0)
    {
        return -1;
    }
    int iID = objpool_alloc(m_pTimerArgPool);
    if (0 > iID)
    {
        return -2;
    }

    TTimerArg* pTimerArg = (TTimerArg*)objpool_get(m_pTimerArgPool, iID);
    if (pTimerArg == NULL)
    {
        return -3;
    }

    pTimerArg->m_Buf = buf;
    int iRet = _StartTimer(iID, expireTime);
    if (iRet < 0)
    {
        return -4;
    }
    return 0;
}


template<class TTimerBuf>
int LiteTimerMgr<TTimerBuf>::UpdateTimer( int id, time_t expireTime )
{
    if (m_pTimerArgPool == NULL || expireTime == 0)
    {
        return -1;
    }

    int iRet = IAmTimerMgr::UpdateTimer(id, expireTime);
    if (iRet < 0)
    {
        return -2;
    }
    return 0;
}


template<class TTimerBuf>
int LiteTimerMgr<TTimerBuf>::StopTimer( int id )
{
    if (m_pTimerArgPool == NULL)
    {
        return -1;
    }

    int iRet = _StopTimer(id);
    if(iRet < 0)
    {
        return -2;
    }
    objpool_free(m_pTimerArgPool, id);
    return 0;
}


template<class TTimerBuf>
int LiteTimerMgr<TTimerBuf>::TickOneExpireTimer( time_t now )
{
    TTimerBuf buf;
    return TickOneExpireTimer(now, buf);
}



template<class TTimerBuf>
int LiteTimerMgr<TTimerBuf>::TickOneExpireTimer( time_t now, TTimerBuf & arg )
{
    if (m_pTimerArgPool == NULL)
    {
        return -1;
    }

    int iID = _TickOneExpireTimer(now);
    if (iID < 0)
    {
        return -1;
    }
    TTimerArg* pTimerArg = (TTimerArg*)objpool_get(m_pTimerArgPool, iID);
    if (pTimerArg == NULL)
    {
        return -1;
    }
    arg = pTimerArg->m_Buf;
    objpool_free(m_pTimerArgPool, iID);
    return iID;
}




#endif  // __UTILLOGIC_TIMERMANAGER_LITETIMERMGR_H__
