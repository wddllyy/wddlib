#include <stdlib.h>
#include "obj_pool.h"

int objpool_init( TObjPool ** pObjPool, size_t iMax, size_t iUnit )
{
    if (pObjPool == NULL)
    {
        return ERR_OBJPOOL_NULLPTR;
    }
    
    int iMemCost = GET_POOL_MEM_SIZE(iUnit, iMax);
    *pObjPool = (TObjPool *)malloc(iMemCost);
    if (*pObjPool == NULL)
    {
        return ERR_OBJPOOL_MEMOVER;
    }


    TObjPool * pData = *pObjPool;
    pData->m_iCapacity = iMax;
    pData->m_iFreeHead = 0;
    pData->m_iUsedCount = 0;
    pData->m_iUsedHead = -1;
    pData->m_iFreeTail = iMax - 1;
    pData->m_iUnitSize = iUnit;
    pData->m_pIdx = (TPoolIdx *)((char *)pData + sizeof(TObjPool));
    pData->m_pObj = ((char *)pData + sizeof(TObjPool) + iMax*sizeof(TPoolIdx));

    for (size_t i = 0 ; i < iMax; ++i)
    {
        pData->m_pIdx[i].m_iNextIdx = i + 1;
        pData->m_pIdx[i].m_iPrevIdx = i - 1;
        pData->m_pIdx[i].m_iAccumulate = 0;
        pData->m_pIdx[i].m_iUsedFlag = 0;
    }

    pData->m_pIdx[iMax - 1].m_iNextIdx = -1;
    return 0;
}

int objpool_destroy( TObjPool ** pObjPool )
{
    if (pObjPool != NULL && * pObjPool != NULL)
    {
        free(* pObjPool);
        * pObjPool = NULL;
    }
    return 0;
}

void* objpool_get( TObjPool* pObjPool, int iIdx )
{
    if (pObjPool == NULL)
    {
        return NULL;
    }

    int iRealIdx = iIdx % pObjPool->m_iCapacity;
    if ( iRealIdx >= pObjPool->m_iCapacity || iRealIdx < 0 || pObjPool->m_iUsedCount <= 0 )
    {
        return NULL;
    }


    int iAccumulate = pObjPool->m_pIdx[iRealIdx].m_iAccumulate;
    int iBoundCheckIdx = pObjPool->m_iCapacity * iAccumulate + iRealIdx;

    if (iIdx != iBoundCheckIdx)
    {
        return NULL;
    }


    if (pObjPool->m_pIdx[iRealIdx].m_iUsedFlag == 0)
    {
        return NULL;
    }

    return pObjPool->m_pObj + iRealIdx * pObjPool->m_iUnitSize;
}

void* objpool_get_bypos( TObjPool* pObjPool, int iPos )
{
    if (pObjPool == NULL)
    {
        return NULL;
    }

    if ( iPos >= pObjPool->m_iCapacity || iPos < 0 || pObjPool->m_iUsedCount <= 0 )
    {
        return NULL;
    }

    return pObjPool->m_pObj + iPos * pObjPool->m_iUnitSize;
}

int objpool_alloc( TObjPool* pObjPool )
{
    if (pObjPool == NULL)
    {
        return ERR_OBJPOOL_NULLPTR;
    }

    if (pObjPool->m_iUsedCount >= pObjPool->m_iCapacity)
    {
        return ERR_OBJPOOL_FULL;
    }

    const int iTempIdx = pObjPool->m_iFreeHead;
    const int iUsedPrevIdx = pObjPool->m_iUsedHead;

    TPoolIdx * pNewIdx = &(pObjPool->m_pIdx[iTempIdx]);
    const int iFreeNextIdx = pNewIdx->m_iNextIdx;

    ////////////////////////////////////////////////////////////////////

    // 空闲头指向当前分配的下一个
    pObjPool->m_iFreeHead = iFreeNextIdx;

    // 更新列表的的首个used
    pObjPool->m_iUsedHead = iTempIdx;

    // 如果是分配的是最后一个了， 更新freetail为无效
    if (pObjPool->m_iFreeHead < 0)
    {
        pObjPool->m_iFreeTail = -1;
    }

    ++pObjPool->m_iUsedCount;
    ////////////////////////////////////////////////////////////////////

    // 更新首个free的的prev为-1
    if (iFreeNextIdx >= 0)
    {
        pObjPool->m_pIdx[iFreeNextIdx].m_iPrevIdx = -1;
    }

    // 更新前一个used的的prev为我
    if (iUsedPrevIdx >= 0)
    {
        pObjPool->m_pIdx[iUsedPrevIdx].m_iPrevIdx = iTempIdx;
    }

    // 修改我的属性
    pNewIdx->m_iUsedFlag = 1;
    pNewIdx->m_iNextIdx = iUsedPrevIdx;
    pNewIdx->m_iPrevIdx = -1;
    ++(pNewIdx->m_iAccumulate);

    int iLogicIdx = pNewIdx->m_iAccumulate * pObjPool->m_iCapacity + iTempIdx;
    if (pNewIdx->m_iAccumulate < 0 || iLogicIdx < 0)
    {
        pNewIdx->m_iAccumulate = 1;
        iLogicIdx = pNewIdx->m_iAccumulate * pObjPool->m_iCapacity + iTempIdx;
    }
    return iLogicIdx;
}

int objpool_free( TObjPool* pObjPool, int iIdx )
{
    if (pObjPool == NULL)
    {
        return ERR_OBJPOOL_NULLPTR;
    }

    int iReadIdx = iIdx % pObjPool->m_iCapacity;
    if ( iReadIdx >= pObjPool->m_iCapacity || iReadIdx < 0 || pObjPool->m_iUsedCount <= 0 )
    {
        return ERR_OBJPOOL_LOGICERROR;
    }

    TPoolIdx * pCurIdx =  &(pObjPool->m_pIdx[iReadIdx]);
    if (pCurIdx->m_iUsedFlag != 1)
    {
        return ERR_OBJPOOL_LOGICERROR;
    }

    int iAccumulate = pCurIdx->m_iAccumulate;
    int iBoundCheckIdx = pObjPool->m_iCapacity * iAccumulate + iReadIdx;

    if (iIdx != iBoundCheckIdx)
    {
        return ERR_OBJPOOL_LOGICERROR;
    }

    int iLastFreeTail = pObjPool->m_iFreeTail;
    int iNextUsedIdx = pCurIdx->m_iNextIdx;
    int iPrevUsedIdx = pCurIdx->m_iPrevIdx;

    //////////////////////////////////////////////////////////
    // FreeTail指向我
    pObjPool->m_iFreeTail = iReadIdx;

    //如果我释放前队列都已经被分配出去
    if (iLastFreeTail < 0)
    {
        pObjPool->m_iFreeHead = iReadIdx;
    }

    //如果我是usedhead，则更新usedhead
    if (pObjPool->m_iUsedHead == iReadIdx)
    {
        pObjPool->m_iUsedHead = iNextUsedIdx;
    }

    --pObjPool->m_iUsedCount;
    //////////////////////////////////////////////////////////
    if (iPrevUsedIdx >= 0)
    {
        pObjPool->m_pIdx[iPrevUsedIdx].m_iNextIdx = pCurIdx->m_iNextIdx;
    }
    if (iNextUsedIdx >= 0)
    {
        pObjPool->m_pIdx[iNextUsedIdx].m_iPrevIdx = pCurIdx->m_iPrevIdx;
    }

    if (iLastFreeTail >= 0)
    {
        pObjPool->m_pIdx[iLastFreeTail].m_iNextIdx = iReadIdx;
    }
    //////////////////////////////////////////////////////////
    // 修改我的属性
    pCurIdx->m_iUsedFlag = 0;
    pCurIdx->m_iNextIdx = -1;
    pCurIdx->m_iPrevIdx = iLastFreeTail;

    return 0;
}

int objpool_free_bypos( TObjPool* pObjPool, int iPos )
{
    if (pObjPool == NULL)
    {
        return ERR_OBJPOOL_NULLPTR;
    }

    if ( iPos >= pObjPool->m_iCapacity || iPos < 0 || pObjPool->m_iUsedCount <= 0 )
    {
        return ERR_OBJPOOL_LOGICERROR;
    }

    TPoolIdx * pCurIdx = &(pObjPool->m_pIdx[iPos]);
    int iLogicIdx = pObjPool->m_iCapacity * pCurIdx->m_iAccumulate + iPos;

    return objpool_free(pObjPool, iLogicIdx);
}

size_t objpool_get_used_items( TObjPool* pObjPool )
{
    if (pObjPool == NULL)
    {
        return ERR_OBJPOOL_NULLPTR;
    }
    return pObjPool->m_iUsedCount;
}

size_t objpool_get_size( TObjPool *pObjPool )
{
    if (pObjPool == NULL)
    {
        return ERR_OBJPOOL_NULLPTR;
    }
    return pObjPool->m_iCapacity;
}

int objpool_travel_used( TObjPool *pObjPool, OBJPOOL_TRAVEL_FUNC pfnTravel, void* pvArg )
{
    if (pObjPool == NULL)
    {
        return ERR_OBJPOOL_NULLPTR;
    }

    for (int i = pObjPool->m_iUsedHead ; i != -1 ; )
    {
        TRAVEL_RET ret = pfnTravel(objpool_get_bypos(pObjPool, i), pvArg);
        if (ret == BREAK_TRAVEL)
        {
            break;
        }
        else if(ret == REMOVE_ME )
        {
            int oldi = i;
            i =  pObjPool->m_pIdx[i].m_iNextIdx;
            objpool_free_bypos(pObjPool, oldi);
        }
        else
        {
            i =  pObjPool->m_pIdx[i].m_iNextIdx;
        }
        
    }
    return 0;
}




