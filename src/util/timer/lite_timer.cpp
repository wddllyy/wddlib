#include "util/timer/lite_timer.h"




IAmTimerMgr::IAmTimerMgr()
{
    Reset();
}


IAmTimerMgr::~IAmTimerMgr()
{
    Reset();
}

int IAmTimerMgr::Reset()
{
    m_TimerHeap.clear();
    return 0;
}

int IAmTimerMgr::_StartTimer( int id, time_t expireTime )
{
    int idx = m_TimerHeap.size();
    m_TimerHeap.push_back(LiteTimer(id, idx, expireTime, *this));
    SetTimerIdxById(id, idx);

    std::push_heap(m_TimerHeap.begin(), m_TimerHeap.end());
    return 0;
}

int IAmTimerMgr::UpdateTimer( int id, time_t expireTime )
{
    int idx = GetTimerIdxByID(id);
    if(idx < 0 || m_TimerHeap.size() < 1 || expireTime == 0)
    {
        return -1;
    }
    m_TimerHeap[idx].m_expireTime = 0;

    std::push_heap(m_TimerHeap.begin(), m_TimerHeap.begin()+idx+1);
    std::pop_heap(m_TimerHeap.begin(), m_TimerHeap.end());
    m_TimerHeap[m_TimerHeap.size() - 1].m_expireTime = expireTime;

    std::push_heap(m_TimerHeap.begin(), m_TimerHeap.end());
    return 0;
}

int IAmTimerMgr::_StopTimer( int id )
{
    int idx = GetTimerIdxByID(id);
    if(idx < 0 || m_TimerHeap.size() < 1)
    {
        return -1;
    }

    m_TimerHeap[idx].m_expireTime = 0;

    std::push_heap(m_TimerHeap.begin(), m_TimerHeap.begin()+idx+1);
    std::pop_heap(m_TimerHeap.begin(), m_TimerHeap.end());
    m_TimerHeap.pop_back();
    return 0;
}

int IAmTimerMgr::_TickOneExpireTimer( time_t now )
{
    if(m_TimerHeap.size() != 0 && m_TimerHeap[0].m_expireTime <= now)
    {
        int id = m_TimerHeap[0].m_id;
        std::pop_heap(m_TimerHeap.begin(), m_TimerHeap.end());
        m_TimerHeap.pop_back();
        return id;
    }

    return -1;
}


