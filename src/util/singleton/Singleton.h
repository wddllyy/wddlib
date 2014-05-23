/************************************************************************ 
 * Copyright(C) zcye 2011
 ***********************************************************************/
#ifndef SRC_UTIL_SINGLETON_H_
#define SRC_UTIL_SINGLETON_H_


template<typename T , bool t >
struct _IFTHENELSE;

template<typename T>
class SingletonHolder
{
public:
    friend struct _IFTHENELSE<T, true>;
    friend struct _IFTHENELSE<T, false>;
    static void SetSingleton(T* pT);
    static T* GetInstance();
    static void Release();
    class SHGC
    {
    public:
        SHGC()
        {}
        ~SHGC()
        {
            SingletonHolder<T>::Release();
        }
    };
private:
    SingletonHolder();
    ~SingletonHolder();
    SingletonHolder<T>& operator=( const SingletonHolder<T>& );
    static T* m_pHoldee;
};
#include "Singleton.hpp"

#endif  // SRC_UTIL_SINGLETON_H_
