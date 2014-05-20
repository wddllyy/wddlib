#include <memory.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#ifndef WIN32
#include <sys/resource.h>
#endif
#include "util/coroutine/comgr.h"


SCoMgr::SCoMgr()
{
	m_WorkMode = FIXED_STACKSIZE_POOL;
    m_pCoPool = NULL;
    m_iDefaultStackSize = 0;
    m_iArgSize = 0;
    m_iRunningID = 0;

    m_Sch.allocfunc = NULL;
    m_Sch.freefunc = NULL;
    m_Sch.pRunCo = NULL;
    m_Sch.isDynamicStack = false;
    m_Sch.ctx.uc_stack.ss_sp = NULL;
    m_Sch.ctx.uc_stack.ss_size = 0;
}

SCoMgr::~SCoMgr()
{
    Destory();
}



int SCoMgr::InitCoMgr(int iArgSize ,int iStackSize, int iPoolSize, CoMgrWorkMode iMode, CoStackMemAlloc allocFunc, CoStackMemFree freeFunc)
{
    if (m_pCoPool != NULL)
    {
        return -1;
    }
    int iPoolUnitSize = sizeof(SCoroutine) + iArgSize;
    if (iMode == FIXED_STACKSIZE_POOL)
    {
        iPoolUnitSize = sizeof(SCoroutine) + iArgSize + iStackSize;
    }
    int iRet = objpool_init(&m_pCoPool, iPoolSize, iPoolUnitSize);
    if (iRet < 0)
    {
        return -2;
    }
    //printf("sss %d %d %d %d\n",iPoolSize, iPoolUnitSize ,(int)sizeof(SCoroutine) + iArgSize, iMode);
    //while(1){sleep(1);}
    m_iDefaultStackSize = iStackSize;
    m_iArgSize = iArgSize;
    m_WorkMode = iMode;
    m_iRunningID = 0;

    m_Sch.allocfunc = NULL;
    m_Sch.freefunc = NULL;
    m_Sch.pRunCo = NULL;
    m_Sch.isDynamicStack = false;
    m_Sch.ctx.uc_stack.ss_sp = NULL;
    m_Sch.ctx.uc_stack.ss_size = 0;

    switch (m_WorkMode)
    {
    case FIXED_STACKSIZE_POOL:
    	break;
    case FIXED_STACKSIZE_MALLOC:

        m_Sch.allocfunc = allocFunc;
        m_Sch.freefunc = freeFunc;
        break;
    case DYNAMIC_STACKSIZE_MALLOC:

        m_Sch.allocfunc = allocFunc;
        m_Sch.freefunc = freeFunc;
        m_Sch.isDynamicStack = true;

        // 获取系统的栈大小，作为运行时主栈大小
        // 区分m_iDefaultStackSize。m_iDefaultStackSize是用来在生成协程时默认的协程保存栈大小，如果运行时该大小不够，会自动扩展
        struct rlimit rl;
        iRet = getrlimit(RLIMIT_STACK, &rl);
        if (iRet != 0)
        {
            return -3;
        }

        m_Sch.firststack = (char *)m_Sch.allocfunc(rl.rlim_cur);
        m_Sch.firststackmaxsize = rl.rlim_cur;
        m_Sch.secondstack = (char *)m_Sch.allocfunc(rl.rlim_cur);
        m_Sch.secondstackmaxsize = rl.rlim_cur;
        break;
    default:
        return -4;
        break;
    }
    return 0;
}




int SCoMgr::Alloc(SCoFunc func, void * arg, int iArgLen)
{
    int iIdx = objpool_alloc(m_pCoPool);
    if (0 > iIdx)
    {
        return -1;
    }

    SCoroutine* pCo = GetCo(iIdx);
    if (pCo == NULL)
    {
        return -2;
    }
    
    pCo->iPoolID = iIdx;
    pCo->tExpireTime = 0;
    pCo->isTimeOut = false;


    memcpy(pCo->m_Buf, arg, m_iArgSize);
    char * pStack = NULL;
    int iStackSize = 0;

    switch (m_WorkMode)
    {
    case FIXED_STACKSIZE_POOL:
        pStack = &(pCo->m_Buf[m_iArgSize]);
        iStackSize = m_iDefaultStackSize;
        break;
    case FIXED_STACKSIZE_MALLOC:
        pStack = (char *)m_Sch.allocfunc(m_iDefaultStackSize);
        iStackSize = m_iDefaultStackSize;
        break;
    case DYNAMIC_STACKSIZE_MALLOC:
        pStack = (char *)m_Sch.firststack;
        iStackSize = m_Sch.firststackmaxsize;
        break;
    default:
        return -4;
        break;
    }

    int iRet = co_init(&pCo->m_Co, (coroutine_func)func, (void *)iIdx, pStack, iStackSize, &m_Sch);
    if (iRet < 0)
    {
        return -5;
    }

    if (m_WorkMode == DYNAMIC_STACKSIZE_MALLOC && m_iDefaultStackSize != 0)
    {
        pCo->m_Co.stack = (char *)m_Sch.allocfunc(m_iDefaultStackSize);
        pCo->m_Co.stackmaxsize = m_iDefaultStackSize;
    }
    return iIdx;
}

SCoroutine* SCoMgr::GetCo( int iPoolID )
{
    SCoroutine* pCo= (SCoroutine*)objpool_get(m_pCoPool, iPoolID);
    if (NULL == pCo)
    {
        return NULL;
    }
    return pCo;
}

int SCoMgr::Release( int iPoolID )
{
    //printf("Release iPoolID:%d\n", iPoolID);
    SCoroutine* pCo= (SCoroutine*)objpool_get(m_pCoPool, iPoolID);
    if (NULL == pCo)
    {
        return -1;
    }
    if (m_WorkMode == DYNAMIC_STACKSIZE_MALLOC)
    {
        if (pCo->m_Co.stack != NULL && m_Sch.freefunc != NULL)
        {
            m_Sch.freefunc(pCo->m_Co.stack);
            pCo->m_Co.stack = NULL;
            pCo->m_Co.stacksize = 0;
            pCo->m_Co.stackmaxsize = 0;
        }
    }
    else if(m_WorkMode == FIXED_STACKSIZE_MALLOC)
    {
        if (pCo->m_Co.ctx.uc_stack.ss_sp != NULL && m_Sch.freefunc != NULL)
        {
            m_Sch.freefunc(pCo->m_Co.ctx.uc_stack.ss_sp);
            pCo->m_Co.ctx.uc_stack.ss_sp = NULL;
            pCo->m_Co.ctx.uc_stack.ss_size = 0;
        }
    }

    return objpool_free(m_pCoPool, iPoolID);
}

int SCoMgr::Destory()
{
    // TODO : 考虑是否先释放掉所有仍存在的coroutine
    if (m_WorkMode == DYNAMIC_STACKSIZE_MALLOC && m_Sch.freefunc != NULL)
    {
        //printf("%s:%d\n", __FUNCTION__, __LINE__);
        if(m_Sch.ctx.uc_stack.ss_sp != NULL)
        {
            m_Sch.freefunc(m_Sch.ctx.uc_stack.ss_sp);
            m_Sch.ctx.uc_stack.ss_sp = NULL;
        }
        m_Sch.ctx.uc_stack.ss_size = 0;
    }
    m_Sch.allocfunc = NULL;
    m_Sch.freefunc = NULL;
    m_Sch.pRunCo = NULL;
    m_Sch.isDynamicStack = false;

    if (m_pCoPool != NULL)
    {
        objpool_destroy(&m_pCoPool);
    }
    m_pCoPool = NULL;
	m_iDefaultStackSize = 0;
    m_iArgSize = 0;
    m_iRunningID = 0;
    m_WorkMode = FIXED_STACKSIZE_POOL;
    return 0;
}

int SCoMgr::Resume( int iPoolID )
{
    SCoroutine* pCo = GetCo(iPoolID);
    if (pCo == NULL)
    {
        return -1;
    }
    
    m_iRunningID = iPoolID;
    
    int iRet = co_resume(&pCo->m_Co, &m_Sch);
    if (iRet < 0)
    {
        Release(iPoolID);
        return -2;
    }

    
    m_iRunningID = 0;
    

    if (pCo->m_Co.status == CO_STATE_END)
    {
        Release(iPoolID);
        return 0;
    }

    return iPoolID;
}

int SCoMgr::Yield(time_t expiretime)
{
    SCoroutine* pCo = GetCo(m_iRunningID);
    if (pCo == NULL)
    {
        printf("pCo : %d == NULL \n",m_iRunningID);
        return -1;
    }
    pCo->tExpireTime = expiretime;
    pCo->isTimeOut = false;
    //m_iRunningID = 0;
    co_yield(&m_Sch);
    if (pCo->isTimeOut)
    {
        return YIELD_TIMEOUT;
    }
    return 0;
}

void* SCoMgr::GetArg( int iPoolID )
{
    SCoroutine* pCo = GetCo(iPoolID);
    if (pCo == NULL)
    {
        return NULL;
    }
    return pCo->m_Buf;
}
struct ProExpireArg
{
    time_t tNow;
    SCoMgr* pMgr;
};
static TRAVEL_RET fnProcessExpire(void* pvNode, void* pvArg)
{
    if (pvNode == NULL || pvArg == NULL)
    {
        return DO_THING;
    }

    SCoroutine* pCo = (SCoroutine*)pvNode;
    ProExpireArg* pArg = (ProExpireArg *)pvArg;
    time_t tNow = pArg->tNow;

    if (pCo->tExpireTime <= tNow && pArg->pMgr)
    {
        pCo->isTimeOut = true;
        pArg->pMgr->Resume(pCo->iPoolID);
    }
    return DO_THING;
}


int SCoMgr::ProcessAllExpire( time_t tNow )
{
    ProExpireArg arg;
    arg.tNow = tNow;
    arg.pMgr = this;
    return objpool_travel_used(m_pCoPool, fnProcessExpire, &arg);
}

int SCoMgr::ProcessPartExpire( time_t tNow, int iOnceProcessCount )
{
    static int iLastPos;
    int iMaxCount = objpool_get_size(m_pCoPool);
    if (iLastPos>=iMaxCount)
    {
        iLastPos = 0;
    }

    int iEnd = iLastPos + iOnceProcessCount;
    if (iEnd > iMaxCount)
    {
        iEnd = iMaxCount;
    }

    for (; iLastPos < iEnd; ++iLastPos)
    {
        SCoroutine* pCo= (SCoroutine*)objpool_get_bypos(m_pCoPool, iLastPos);
        if (pCo == NULL)
        {
            continue;
        }

        if (pCo->tExpireTime <= tNow)
        {
            pCo->isTimeOut = true;
            Resume(pCo->iPoolID);
        }
    }
    return 0;
}
static TRAVEL_RET fnProcessfun(void* pvNode, void* pvArg)
{
    if (pvNode == NULL || pvArg == NULL)
    {
        return DO_THING;
    }

    SCoroutine* pCo = (SCoroutine*)pvNode;
    SCoFunc fun = (SCoFunc)pvArg;

    fun(pCo->iPoolID);
    return DO_THING;
}
int SCoMgr::ProcessAllActiveFun( SCoFunc fun )
{
    return objpool_travel_used(m_pCoPool, fnProcessfun, (void*)fun);
}

int SCoMgr::GeCoCount()
{
    return objpool_get_used_items(m_pCoPool);
}

int SCoMgr::GetRunningCoStackSize()
{
    return co_runningstacksize(&m_Sch);
}


