/**
 *  @file     SCoMgr.h
 *  @brief    封装了固定栈大小的coroutine管理器，单SCoMgr管理一组固定协程栈大小和固定协程函数参数大小
              基于单schedule多task模型
              参数空间是用于存储coroutine函数的参数，单独开空间用于重入。
              栈空间用于coroutine函数的栈存储，注意使用时不要使用超大栈变量或者高深度递归函数调用，否则可能会超出设定的栈空间，而导致栈溢出
              目前不支持共享内存恢复，主要是因为应用场景受限， 1 栈上跨yield不能使用指针 2 版本更新时，栈上数据分布很难保持兼容

              两个模式： 定长栈和变长栈
              定长栈：调度器的每个coroutine 固定长度栈
              变长栈：调度器的每个coroutine 栈长度变长，自适应于该coroutine使用栈空间的多少。有内存拷贝，栈使用越多消耗越大。
                      变长栈模式下，所有coroutine的运行栈都在调度器上共享一个，该共享栈大小位系统设定getrlimit， 可使用命令ulimit -a来查看和修改

 *  @date     2013-10-29
*/



#ifndef __UTIL_SCOMGR_H
#define __UTIL_SCOMGR_H
#include "util/coroutine/coroutine.h"
#include "util/container/obj_pool.h"

// 封装使用的协程函数类型，参数为Coroutine的id，可以使用SCoMgr::GetArg获取参数空间
typedef int (*SCoFunc)(int iCoPoolID);
const int YIELD_TIMEOUT = -987654321;

// 协程结构
struct SCoroutine
{
    coroutine m_Co;                 // 内部coroutine结构
    int iPoolID;                    // 在SCoMgr中管理的id
    time_t tExpireTime;             // 过期时间
    bool isTimeOut;
    //总长度为iArgSize + iStackSize
    char m_Buf[0];                  // 定制参数空间+栈空间的定长内存基址
};

enum CoMgrWorkMode
{
    FIXED_STACKSIZE_POOL = 0,  //定长栈，栈空间在SCoMgr的pool中
    FIXED_STACKSIZE_MALLOC = 1,  //定长栈，栈空间由CoStackMemAlloc生成
    DYNAMIC_STACKSIZE_MALLOC = 2, //变长栈，栈空间CoStackMemAlloc生成
};

// 定长栈空间协程管理器
class SCoMgr
{
public:
    SCoMgr();
    ~SCoMgr();
public:
    /***
    *  @brief   协程管理器初始化函数
    *  @param   iArgSize: 参数空间长度，参数空间是用于存储coroutine函数的参数，单独开空间用于重入。
    *  @param   iStackSize: 栈空间长度，用于coroutine函数的栈存储，注意使用时不要使用超大栈变量或者高深度递归函数调用，否则可能会超出设定的栈空间，而导致栈溢出
    *  @param   iPoolSize: 管理器池容量
    *  @param   iMode: 管理器模式
    *  @param   allocFunc: 栈空间分配器。
    *  @param   freeFunc: 栈空间回收器
    *  @return  等于0: 成功
                小于0: 失败
    ***/
    int InitCoMgr(int iArgSize ,int iStackSize, int iPoolSize, CoMgrWorkMode iMode, CoStackMemAlloc allocFunc, CoStackMemFree freeFunc);

    /***
    *  @brief   协程管理器销毁函数
    ***/
    int Destory();

    /***
    *  @brief   协程分配函数，协程执行结束或者SCoMgr托管超时的情况下超时后会自动销毁，若自行管理超时需要自行调用Release，时间复杂度O(1)
    *  @param   func: 协程绑定函数。
    *  @param   arg: 协程参数变量地址，内部会做一次拷贝(memcpy)，拷贝至该coroutine的参数空间
    *  @param   iArgLen: 参数变量长度
    *  @return  大于0: 成功，值代表了分配的协程的id，该id等于对应SCoroutine::iPoolID
                小于0: 失败
    ***/
    int Alloc(SCoFunc func, void * arg, int iArgLen);



    /***
    *  @brief   释放SCoroutine，在执行结束后或者托管超时模式下超时后自动被调用，若自行管理超时则需要显式调用， 时间复杂度O(1)
    *  @param   iPoolID: SCoroutine的id，通过Alloc获得。
    *  @return  等于0: 成功
                小于0: 失败
    ***/
    int Release(int iPoolID);


    /***
    *  @brief   获取对应协程的参数，一般用于协程自己读写或者其他协程与其数据交互
    *  @param   iPoolID: SCoroutine的id，通过Alloc获得。
    *  @return  不等于NULL: 成功，返回该协程的参数空间地址
                等于0: 失败
    ***/
    void* GetArg(int iPoolID);

    /***
    *  @brief   检查所有分配协程是否超时，会遍历检查所有超时，不需要频繁调用，
    *  @param   tNow: 当前时间。
    *  @return  等于0: 成功
                小于0: 失败
    ***/
    int ProcessAllExpire(time_t tNow);

    /***
    *  @brief   遍历检查所有协程(分配+未分配)，一次只检查iOnceProcessCount个
    *  @param   tNow: 当前时间。
    *  @param   iOnceProcessCount: 一次检查数量。
    *  @return  等于0: 成功
                小于0: 失败
    ***/
    int ProcessPartExpire(time_t tNow, int iOnceProcessCount);


    /***
    *  @brief   遍历所有分配的协程，并调用传入的函数
    *  @param   fun: 对每个协程需要执行的函数。
    *  @return  等于0: 成功
                小于0: 失败
    ***/
    int ProcessAllActiveFun(SCoFunc fun);

    /***
    *  @brief   返回分配的协程的数量
    *  @return  返回分配的协程的数量
    ***/
    int GeCoCount();


    /***
    *  @brief   通过id获取SCoroutine，时间复杂度O(1)
    *  @param   iPoolID: SCoroutine的id，通过Alloc获得。
    *  @return  不等于NULL: 成功
                等于NULL: 失败
    ***/
    SCoroutine* GetCo(int iPoolID);

    int GetRunningCoID()
    {
        return m_iRunningID;
    }

    int GetRunningCoStackSize();
public:
    /***
    *  @brief   赋予对应协程的执行权， 在开启协程或者异步操作结束后调用
    *  @param   iPoolID: SCoroutine的id，通过Alloc获得。
    *  @return  等于0: 成功，并且该协程的对应函数执行结束
                大于0: 成功，返回iPoolID
                小于0: 失败
    ***/
    int Resume(int iPoolID);

    /***
    *  @brief   挂起当前正在执行的协程，在协程函数里调用，目前设计为不用传id参数，因为同时执行的协程只有一个，所以管理器内部保存了(因此多一个状态值)
    *  @return  等于0: 成功，挂起正常返回
                小于0: 失败
                == YIELD_TIMEOUT : 该挂起导致coroutine整体超时
    ***/
    int Yield(time_t expiretime);

private:


private:
    CoMgrWorkMode      m_WorkMode;
    TObjPoolPtr         m_pCoPool;
    int m_iDefaultStackSize;
    int m_iArgSize;
    int m_iRunningID;

    schedule m_Sch;
};


#endif
