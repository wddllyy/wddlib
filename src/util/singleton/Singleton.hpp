/************************************************************************ 
 * Copyright(C) zcye 2011
 ***********************************************************************/
#ifndef SRC_UTIL_DETAIL_SINGLETON_HPP_
#define SRC_UTIL_DETAIL_SINGLETON_HPP_

#include <assert.h>
#include <stdio.h>


template<typename T , void (T::*_singleton_init) ()>
struct _Mid;

template<typename T >
static char CheckFun(_Mid<T , &T::_singleton_init>* );

template<typename T >
static int CheckFun(...);

template<typename T >
struct IS_Need_singleton_init
{
    enum
    {
        result = (sizeof(CheckFun<T >( NULL )) == sizeof(char)),
    };
};

/// _IFTHENELSE

template<typename T , bool t >
struct _IFTHENELSE
{
    static void OP(T* pT );
};

template<typename T >
struct _IFTHENELSE<T , true>
{
    static void OP(T* pT  )
    {
        assert(pT != NULL);

        if ( SingletonHolder<T>::m_pHoldee == NULL )
        {
            SingletonHolder<T>::m_pHoldee = pT;
            SingletonHolder<T>::m_pHoldee->_singleton_init();
        }
    }
};

template<typename T  >
struct _IFTHENELSE<T , false>
{
    static void OP (T* pT )
    {
        assert(pT != NULL);

        if ( SingletonHolder<T>::m_pHoldee == NULL )
        {
            SingletonHolder<T>::m_pHoldee = pT;
        }
    }
};

/// SingletonHolder

template<typename T>
T* SingletonHolder<T>::m_pHoldee = NULL;

template<typename T>
void SingletonHolder<T>::SetSingleton(T* pT)
{
    static SHGC GarbageCollector;//垃圾回收机制
    _IFTHENELSE<T, IS_Need_singleton_init<T>::result>::OP(pT);
}

template<typename T>
T* SingletonHolder<T>::GetInstance()
{
    return m_pHoldee;
}

template<typename T>
void SingletonHolder<T>::Release()
{
    delete m_pHoldee;
    m_pHoldee = NULL;
}


#endif  // SRC_UTIL_DETAIL_SINGLETON_HPP_
