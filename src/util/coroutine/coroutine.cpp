#include "coroutine.h"
#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <assert.h>
#include <stddef.h>
#include <string.h>
#include <stdint.h>

extern "C"
{
    extern void co_swapcontext(ucontext_t *, ucontext_t *) asm("co_swapcontext");
}

static void _save_stack(coroutine* pCo, schedule* pSch)
{
    char stacknow = 0;
    char* pstack = (char*)pSch->firststack;
    int stacksize = (int)pSch->firststackmaxsize;

    if (pCo->usesecondstack)
    {
        pstack = (char*)pSch->secondstack;
        stacksize = (int)pSch->secondstackmaxsize;
    }

    char* top =  pstack + stacksize;

    //printf("%s:%d co:%p sp:%p size:%d\n", __FUNCTION__, __LINE__, pCo, pstack, (int)stacksize);
    //printf("%s:%d top:%p now:%p\n", __FUNCTION__, __LINE__, top, &stacknow);
    int ssize = top - &stacknow;
    if (pCo->stackmaxsize < ssize)
    {
        //printf("%s:%d\n", __FUNCTION__, __LINE__);
        pSch->freefunc(pCo->stack);
        pCo->stackmaxsize = ssize;
        pCo->stack = (char*)pSch->allocfunc(pCo->stackmaxsize);
    }
    pCo->stacksize = ssize;

    //printf("%s:%d pCo:%p pCo->stacksize %d pSch->ctx.uc_stack.ss_size %d\n", __FUNCTION__, __LINE__, pCo, pCo->stacksize,(int)stacksize);
    memcpy(pCo->stack, &stacknow, ssize);
}

static void _restore_stack(coroutine* pCo, schedule* pSch)
{
    char* pstack = (char*)pSch->firststack;
    int stacksize = (int)pSch->firststackmaxsize;

    if (pCo->usesecondstack)
    {
        pstack = (char*)pSch->secondstack;
        stacksize = (int)pSch->secondstackmaxsize;
    }

    char* top =  pstack + stacksize;
    memcpy(top - pCo->stacksize, pCo->stack, pCo->stacksize);
}
static void _set_dynamic_stack(coroutine* pCo, schedule* pSch, bool issecond)
{
    if(!issecond)
    {
        pCo->usesecondstack = false;
        pCo->ctx.uc_stack.ss_sp = (char*)pSch->firststack;
        pCo->ctx.uc_stack.ss_size = pSch->firststackmaxsize;
    }
    else
    {
        pCo->usesecondstack = true;
        pCo->ctx.uc_stack.ss_sp = (char*)pSch->secondstack;
        pCo->ctx.uc_stack.ss_size = pSch->secondstackmaxsize;
    }

}

static char* get_runningstack(coroutine* pCo, schedule* pSch)
{
    char* pstack = (char*)pSch->firststack;
    if (pCo->usesecondstack)
    {
        pstack = (char*)pSch->secondstack;
    }
    return pstack;
}
static int get_runningstacksize(coroutine* pCo, schedule* pSch)
{
    int stacksize = (int)pSch->firststackmaxsize;
    if (pCo->usesecondstack)
    {
        stacksize = (int)pSch->secondstackmaxsize;
    }
    return stacksize;
}

int co_init(coroutine* pCo, coroutine_func func, void* arg, char * pStack, size_t iStackSize, schedule*  sch)
{
    if (pCo == NULL || sch == NULL)
    {
        return -1;
    }

    pCo->func = func;
    pCo->arg = arg;
    pCo->status = CO_STATE_READY;
    pCo->ctx.uc_stack.ss_sp = pStack;
    pCo->ctx.uc_stack.ss_size = iStackSize;
    pCo->ctx.uc_link = &sch->ctx;
    pCo->stacksize = 0;
    pCo->stack = NULL;
    pCo->stackmaxsize = 0;
    pCo->usesecondstack = false;
    return 0;
}
//On architectures where int and pointer types are the same size (e.g., x86-32, where both types are 32 bits), 
//you may be able to get away with passing pointers as arguments to makecontext() following argc. 
//However, doing this is not guaranteed to be portable, is undefined according to the standards, 
//and won't work on architectures where pointers are larger than ints. 
//Nevertheless, starting with version 2.8, glibc makes some changes to makecontext(3), 
//to permit this on some 64-bit architectures (e.g., x86-64).
static void _process(uint32_t low32, uint32_t hi32) 
{
    uintptr_t ptr = (uintptr_t)low32 | ((uintptr_t)hi32 << 32);
    schedule* pSch = (schedule *)ptr;
    if (pSch == NULL || pSch->pRunCo == NULL)
    {
        return;
    }
    coroutine* pCo = pSch->pRunCo;

    //printf("%s:%d\n", __FUNCTION__, __LINE__);
    pCo->func(pCo->arg);
    //printf("%s:%d\n", __FUNCTION__, __LINE__);
   

    //if (pSchedule->isDynamicStack)
    //{
    //    printf("%s:%d\n", __FUNCTION__, __LINE__);
    //}

    // 这个swap是因为使用汇编优化版本的swapcontext才需要调用的，否则会core
    //_yield_co(pCo, pSchedule, CO_STATE_END);
    pCo->status = CO_STATE_END;
    pSch->pRunCo = pCo->parent;
    if (pCo->parent != NULL)
    {
        static coroutine* spCo;
        static coroutine* spAwakeCo;
        spCo = pCo;
        spAwakeCo = pCo->parent;
        pCo->ctx.uc_link = &spAwakeCo->ctx;
        // 子协程运行结束之后就脱离和父协程的关系
        pCo->parent = NULL;
        spAwakeCo->status = CO_STATE_RUN;

        //printf("%s:%d pCo:%p pCo->stacksize %d pSch->ctx.uc_stack.ss_size %d\n", __FUNCTION__, __LINE__, spAwakeCo, spAwakeCo->stacksize,(int)get_runningstacksize(spAwakeCo,pSch));
        if (pSch->isDynamicStack)
        {
            // memcpy之后栈内存已经覆盖为spAwakeCo，不可以再访问栈上变量
            //printf("%s:%d\n", __FUNCTION__, __LINE__);
            _restore_stack(spAwakeCo, pSch);
            //printf("%s:%d\n", __FUNCTION__, __LINE__);
        }
        //printf("%s:%d\n", __FUNCTION__, __LINE__);
        co_swapcontext(&spCo->ctx, &spAwakeCo->ctx);       
        //printf("%s:%d\n", __FUNCTION__, __LINE__);
    }
    else
    {
        //printf("%s:%d\n", __FUNCTION__, __LINE__);
        co_swapcontext(&pCo->ctx, &pSch->ctx);
    }

}

int co_resume(coroutine* pCo, schedule* pSch)
{
    if (pCo == NULL || pSch == NULL)
    {
        return -1;
    }
    pCo->parent = NULL;
    if (pSch->pRunCo != NULL)
    {
        pCo->parent = pSch->pRunCo;
        pCo->parent->status = CO_STATE_SUSPEND;
        if (pSch->isDynamicStack)
        {
            //printf("%s:%d\n", __FUNCTION__, __LINE__);
            _save_stack(pCo->parent, pSch);
            //printf("%s:%d\n", __FUNCTION__, __LINE__);
        }

       //_set_use_secondstack(pCo, pSch, !pCo->parent->usesecondstack);
    }
    pSch->pRunCo = pCo;

    //printf("%s:%d %p\n", __FUNCTION__, __LINE__, pCo);
    switch(pCo->status) 
    {
    case CO_STATE_READY:
        pCo->status = CO_STATE_RUN;
        if (pSch->isDynamicStack)
        {
            if (pCo->parent != NULL)
            {
                _set_dynamic_stack(pCo, pSch, !pCo->parent->usesecondstack);
            }
            else
            {
                _set_dynamic_stack(pCo, pSch, false);
            }
        }
        

        getcontext(&pCo->ctx);
        makecontext(&pCo->ctx, (void (*)(void)) _process, 2, (uint64_t)pSch, (uint64_t)pSch>>32);

        //printf("%s:%d\n", __FUNCTION__, __LINE__);
        if (pCo->parent != NULL)
        {
            //printf("%s:%d\n", __FUNCTION__, __LINE__);
            co_swapcontext(&pCo->parent->ctx, &pCo->ctx);
        }
        else
        {
            co_swapcontext(&pSch->ctx, &pCo->ctx);
        }
        //printf("%s:%d\n", __FUNCTION__, __LINE__);
        break;
    case CO_STATE_SUSPEND:
        pCo->status = CO_STATE_RUN;

        //printf("%s:%d pCo:%p pCo->stacksize %d pSch->ctx.uc_stack.ss_size %d\n", __FUNCTION__, __LINE__, pCo, pCo->stacksize,(int)pSch->ctx.uc_stack.ss_size);

        //if (pCo->parent != NULL)
        //{
        //    //printf("ERROR!!\n");
        //}
        static coroutine* spCo;
        static schedule* spSch;
        spCo = pCo;
        spSch = pSch;
        if (pSch->isDynamicStack)
        {
            // memcpy之后栈内存已经覆盖为spAwakeCo，不可以再访问栈上变量
            //memcpy(((char *)pSch->ctx.uc_stack.ss_sp) + pSch->ctx.uc_stack.ss_size - pCo->stacksize, pCo->stack, pCo->stacksize);
            _restore_stack(spCo, spSch);
        }
        if (spCo->parent != NULL)
        {
            printf("error %s:%d %p %d\n", __FUNCTION__, __LINE__, get_runningstack(pCo, pSch), get_runningstacksize(pCo, pSch));
            co_swapcontext(&spCo->parent->ctx, &spCo->ctx);
        }
        else
        {
            //printf("%s:%d\n", __FUNCTION__, __LINE__);
            co_swapcontext(&spSch->ctx, &spCo->ctx);
        }
        

        //if (pCo->status == CO_STATE_END)
        //{
        //    printf("%s:%d pCo:%p\n", __FUNCTION__, __LINE__, pCo);
        //}
        
        
        break;
    default:
        return -2;
    }

    return 0;
}


int co_yield(schedule* pSch)
{
    if (pSch == NULL || pSch->pRunCo == NULL)
    {
        return -1;
    }
    
    coroutine* pCo = pSch->pRunCo;
    //printf("%s:%d\n", __FUNCTION__, __LINE__);
    if (pSch->isDynamicStack)
    {
        _save_stack(pCo, pSch);
    }
    
    //printf("%s:%d\n", __FUNCTION__, __LINE__);
    //_yield_co(pCo, pSch, CO_STATE_SUSPEND);
    pCo->status = CO_STATE_SUSPEND;
    pSch->pRunCo = pCo->parent;
    if (pCo->parent != NULL)
    {
        static coroutine* spAwakeCo = NULL;
        static coroutine* spCo = NULL;
        spCo = pCo;
        spAwakeCo = pCo->parent;
        //printf("%s:%d pCo:%p pCo->stacksize %d pSch->ctx.uc_stack.ss_size %d\n", __FUNCTION__, __LINE__, spAwakeCo, spAwakeCo->stacksize,(int)pSch->ctx.uc_stack.ss_size);
        spAwakeCo->status = CO_STATE_RUN;
        // 子协程运行挂起之后就脱离和父协程的关系
        spCo->parent = NULL;
        //printf("%s:%d\n", __FUNCTION__, __LINE__);
        if (pSch->isDynamicStack)
        {
            // memcpy之后栈内存已经覆盖为spAwakeCo，不可以再访问栈上变量
            // memcpy(((char *)pSch->ctx.uc_stack.ss_sp) + pSch->ctx.uc_stack.ss_size - spAwakeCo->stacksize, spAwakeCo->stack, spAwakeCo->stacksize);
            _restore_stack(spAwakeCo, pSch);
        }
        //printf("%s:%d\n", __FUNCTION__, __LINE__);
        co_swapcontext(&spCo->ctx, &spAwakeCo->ctx);
    }
    else
    {
        //printf("%s:%d\n", __FUNCTION__, __LINE__);
        co_swapcontext(&pCo->ctx, &pSch->ctx);
    }
    
    return 0;
}

int co_runningstacksize( schedule* pSch )
{
    if (pSch == NULL || pSch->pRunCo == NULL)
    {
        return -1;
    }

    char stacknow = 0;
    char* pstack = (char *)pSch->pRunCo->ctx.uc_stack.ss_sp;
    size_t stacksize = pSch->pRunCo->ctx.uc_stack.ss_size;
    if (pSch->isDynamicStack)
    {
        pstack = (char*)pSch->firststack;
        stacksize = (size_t)pSch->firststackmaxsize;

        if (pSch->pRunCo->usesecondstack)
        {
            pstack = (char*)pSch->secondstack;
            stacksize = (size_t)pSch->secondstackmaxsize;
        }
    }

    char* top =  pstack + stacksize;
    int ssize = top - &stacknow;
    return ssize;
}
