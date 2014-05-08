/**
 *  @file     coroutine.h
 *  @brief    coroutine的api封装，使用了libc里的makecontext getcontext， 使用了基于libc的swapcontext优化后该写的co_swapcontext
              基于单schedule多task模型
              目前不支持共享内存恢复，主要是因为应用场景受限， 1 栈上跨yield不能使用指针 2 版本更新时，栈上数据分布很难保持兼容
              两个模式： 定长栈和变长栈
              定长栈：调度器的每个coroutine 固定长度栈
              变长栈：调度器的每个coroutine 栈长度变长，自适应于该coroutine使用栈空间的多少。有内存拷贝，栈使用越多消耗越大。
 *  @date     2013-10-21
*/

#ifndef C_COROUTINE_H
#define C_COROUTINE_H
#include <ucontext.h>

// 协程的状态
enum co_state
{
    CO_STATE_NONE = 0,
    CO_STATE_READY = 1,
    CO_STATE_RUN = 2,
    CO_STATE_SUSPEND = 3,
    CO_STATE_END = 4,
};

// 协程的回调函数类型
typedef int (*coroutine_func)(void* arg);
typedef void* (*CoStackMemAlloc)(size_t uSize);
typedef void (*CoStackMemFree)(void * pMem);

struct coroutine;


struct schedule
{
    ucontext_t ctx;              // libc的context类型
    bool isDynamicStack;    // 是否运行时动态调整coroutine的栈内存
    coroutine * pRunCo;          // 正在运行的coroutine
    CoStackMemAlloc allocfunc;   // 用于动态栈模式，动态栈空间分配函数
    CoStackMemFree freefunc;     // 用于动态栈模式，动态栈空间回收函数
    int secondstackmaxsize;    // 用于动态栈模式，记录动态栈容量
    char * secondstack;        // 用于动态栈模式，记录动态栈空间
    int firststackmaxsize;    // 用于动态栈模式，记录动态栈容量
    char * firststack;        // 用于动态栈模式，记录动态栈空间
};

// 基本协程结构
struct coroutine 
{
	coroutine_func func; // 调用函数指针
	void* arg;           // 调用函数的参数指针
	ucontext_t ctx;      // libc的context类型
	int status;          // 协程的状态
    coroutine * parent;  // 协程的父协程，即yield的时候恢复到父协程的执行
    int stacksize;       // 用于动态栈模式，记录动态栈的当前大小
    int stackmaxsize;    // 用于动态栈模式，记录动态栈容量
    char * stack;        // 用于动态栈模式，记录动态栈空间
    bool usesecondstack;
};


/***
    *  @brief   协程初始化函数
    *  @param   pCo: 需要初始化的协程结构体指针，外部负责创建
    *  @param   func: pCo绑定的函数
    *  @param   arg: 绑定的函数的参数
    *  @param   pStack: 该协程的栈空间地址
    *  @param   iStackSize: 该协程的栈空间大小
    *  @param   sch: 该协程的调度器
    *  @return  等于0: 成功
                小于0: 失败
***/
int co_init(coroutine* pCo, coroutine_func func, void* arg, char * pStack, size_t iStackSize, schedule*  sch);

/***
    *  @brief   协程恢复函数
    *  @param   pCo: 需要恢复的协程结构体指针(一般由schedule负责调用)
    *  @param   sch: 该协程的调度器
    *  @return  等于0: 成功
                小于0: 失败
***/
int co_resume(coroutine* pCo, schedule*  sch);

/***
    *  @brief   协程挂起函数
    *  @param   sch: 该协程的调度器
    *  @return  等于0: 成功
                小于0: 失败
***/
int co_yield(schedule*  sch);


int co_runningstacksize(schedule*  sch);

#endif
