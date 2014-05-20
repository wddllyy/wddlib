/**
 *  @file     SCoMgr.h
 *  @brief    ��װ�˹̶�ջ��С��coroutine����������SCoMgr����һ��̶�Э��ջ��С�͹̶�Э�̺���������С
              ���ڵ�schedule��taskģ��
              �����ռ������ڴ洢coroutine�����Ĳ������������ռ��������롣
              ջ�ռ�����coroutine������ջ�洢��ע��ʹ��ʱ��Ҫʹ�ó���ջ�������߸���ȵݹ麯�����ã�������ܻᳬ���趨��ջ�ռ䣬������ջ���
              Ŀǰ��֧�ֹ����ڴ�ָ�����Ҫ����ΪӦ�ó������ޣ� 1 ջ�Ͽ�yield����ʹ��ָ�� 2 �汾����ʱ��ջ�����ݷֲ����ѱ��ּ���

              ����ģʽ�� ����ջ�ͱ䳤ջ
              ����ջ����������ÿ��coroutine �̶�����ջ
              �䳤ջ����������ÿ��coroutine ջ���ȱ䳤������Ӧ�ڸ�coroutineʹ��ջ�ռ�Ķ��١����ڴ濽����ջʹ��Խ������Խ��
                      �䳤ջģʽ�£�����coroutine������ջ���ڵ������Ϲ���һ�����ù���ջ��Сλϵͳ�趨getrlimit�� ��ʹ������ulimit -a���鿴���޸�

 *  @date     2013-10-29
*/



#ifndef __UTIL_SCOMGR_H
#define __UTIL_SCOMGR_H
#include "util/coroutine/coroutine.h"
#include "util/container/obj_pool.h"

// ��װʹ�õ�Э�̺������ͣ�����ΪCoroutine��id������ʹ��SCoMgr::GetArg��ȡ�����ռ�
typedef int (*SCoFunc)(int iCoPoolID);
const int YIELD_TIMEOUT = -987654321;

// Э�̽ṹ
struct SCoroutine
{
    coroutine m_Co;                 // �ڲ�coroutine�ṹ
    int iPoolID;                    // ��SCoMgr�й����id
    time_t tExpireTime;             // ����ʱ��
    bool isTimeOut;
    //�ܳ���ΪiArgSize + iStackSize
    char m_Buf[0];                  // ���Ʋ����ռ�+ջ�ռ�Ķ����ڴ��ַ
};

enum CoMgrWorkMode
{
    FIXED_STACKSIZE_POOL = 0,  //����ջ��ջ�ռ���SCoMgr��pool��
    FIXED_STACKSIZE_MALLOC = 1,  //����ջ��ջ�ռ���CoStackMemAlloc����
    DYNAMIC_STACKSIZE_MALLOC = 2, //�䳤ջ��ջ�ռ�CoStackMemAlloc����
};

// ����ջ�ռ�Э�̹�����
class SCoMgr
{
public:
    SCoMgr();
    ~SCoMgr();
public:
    /***
    *  @brief   Э�̹�������ʼ������
    *  @param   iArgSize: �����ռ䳤�ȣ������ռ������ڴ洢coroutine�����Ĳ������������ռ��������롣
    *  @param   iStackSize: ջ�ռ䳤�ȣ�����coroutine������ջ�洢��ע��ʹ��ʱ��Ҫʹ�ó���ջ�������߸���ȵݹ麯�����ã�������ܻᳬ���趨��ջ�ռ䣬������ջ���
    *  @param   iPoolSize: ������������
    *  @param   iMode: ������ģʽ
    *  @param   allocFunc: ջ�ռ��������
    *  @param   freeFunc: ջ�ռ������
    *  @return  ����0: �ɹ�
                С��0: ʧ��
    ***/
    int InitCoMgr(int iArgSize ,int iStackSize, int iPoolSize, CoMgrWorkMode iMode, CoStackMemAlloc allocFunc, CoStackMemFree freeFunc);

    /***
    *  @brief   Э�̹��������ٺ���
    ***/
    int Destory();

    /***
    *  @brief   Э�̷��亯����Э��ִ�н�������SCoMgr�йܳ�ʱ������³�ʱ����Զ����٣������й���ʱ��Ҫ���е���Release��ʱ�临�Ӷ�O(1)
    *  @param   func: Э�̰󶨺�����
    *  @param   arg: Э�̲���������ַ���ڲ�����һ�ο���(memcpy)����������coroutine�Ĳ����ռ�
    *  @param   iArgLen: ������������
    *  @return  ����0: �ɹ���ֵ�����˷����Э�̵�id����id���ڶ�ӦSCoroutine::iPoolID
                С��0: ʧ��
    ***/
    int Alloc(SCoFunc func, void * arg, int iArgLen);



    /***
    *  @brief   �ͷ�SCoroutine����ִ�н���������йܳ�ʱģʽ�³�ʱ���Զ������ã������й���ʱ����Ҫ��ʽ���ã� ʱ�临�Ӷ�O(1)
    *  @param   iPoolID: SCoroutine��id��ͨ��Alloc��á�
    *  @return  ����0: �ɹ�
                С��0: ʧ��
    ***/
    int Release(int iPoolID);


    /***
    *  @brief   ��ȡ��ӦЭ�̵Ĳ�����һ������Э���Լ���д��������Э���������ݽ���
    *  @param   iPoolID: SCoroutine��id��ͨ��Alloc��á�
    *  @return  ������NULL: �ɹ������ظ�Э�̵Ĳ����ռ��ַ
                ����0: ʧ��
    ***/
    void* GetArg(int iPoolID);

    /***
    *  @brief   ������з���Э���Ƿ�ʱ�������������г�ʱ������ҪƵ�����ã�
    *  @param   tNow: ��ǰʱ�䡣
    *  @return  ����0: �ɹ�
                С��0: ʧ��
    ***/
    int ProcessAllExpire(time_t tNow);

    /***
    *  @brief   �����������Э��(����+δ����)��һ��ֻ���iOnceProcessCount��
    *  @param   tNow: ��ǰʱ�䡣
    *  @param   iOnceProcessCount: һ�μ��������
    *  @return  ����0: �ɹ�
                С��0: ʧ��
    ***/
    int ProcessPartExpire(time_t tNow, int iOnceProcessCount);


    /***
    *  @brief   �������з����Э�̣������ô���ĺ���
    *  @param   fun: ��ÿ��Э����Ҫִ�еĺ�����
    *  @return  ����0: �ɹ�
                С��0: ʧ��
    ***/
    int ProcessAllActiveFun(SCoFunc fun);

    /***
    *  @brief   ���ط����Э�̵�����
    *  @return  ���ط����Э�̵�����
    ***/
    int GeCoCount();


    /***
    *  @brief   ͨ��id��ȡSCoroutine��ʱ�临�Ӷ�O(1)
    *  @param   iPoolID: SCoroutine��id��ͨ��Alloc��á�
    *  @return  ������NULL: �ɹ�
                ����NULL: ʧ��
    ***/
    SCoroutine* GetCo(int iPoolID);

    int GetRunningCoID()
    {
        return m_iRunningID;
    }

    int GetRunningCoStackSize();
public:
    /***
    *  @brief   �����ӦЭ�̵�ִ��Ȩ�� �ڿ���Э�̻����첽�������������
    *  @param   iPoolID: SCoroutine��id��ͨ��Alloc��á�
    *  @return  ����0: �ɹ������Ҹ�Э�̵Ķ�Ӧ����ִ�н���
                ����0: �ɹ�������iPoolID
                С��0: ʧ��
    ***/
    int Resume(int iPoolID);

    /***
    *  @brief   ����ǰ����ִ�е�Э�̣���Э�̺�������ã�Ŀǰ���Ϊ���ô�id��������Ϊͬʱִ�е�Э��ֻ��һ�������Թ������ڲ�������(��˶�һ��״ֵ̬)
    *  @return  ����0: �ɹ���������������
                С��0: ʧ��
                == YIELD_TIMEOUT : �ù�����coroutine���峬ʱ
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
