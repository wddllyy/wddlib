/**
 *  @file     coroutine.h
 *  @brief    coroutine��api��װ��ʹ����libc���makecontext getcontext�� ʹ���˻���libc��swapcontext�Ż����д��co_swapcontext
              ���ڵ�schedule��taskģ��
              Ŀǰ��֧�ֹ����ڴ�ָ�����Ҫ����ΪӦ�ó������ޣ� 1 ջ�Ͽ�yield����ʹ��ָ�� 2 �汾����ʱ��ջ�����ݷֲ����ѱ��ּ���
              ����ģʽ�� ����ջ�ͱ䳤ջ
              ����ջ����������ÿ��coroutine �̶�����ջ
              �䳤ջ����������ÿ��coroutine ջ���ȱ䳤������Ӧ�ڸ�coroutineʹ��ջ�ռ�Ķ��١����ڴ濽����ջʹ��Խ������Խ��
 *  @date     2013-10-21
*/

#ifndef C_COROUTINE_H
#define C_COROUTINE_H
#include <ucontext.h>

// Э�̵�״̬
enum co_state
{
    CO_STATE_NONE = 0,
    CO_STATE_READY = 1,
    CO_STATE_RUN = 2,
    CO_STATE_SUSPEND = 3,
    CO_STATE_END = 4,
};

// Э�̵Ļص���������
typedef int (*coroutine_func)(void* arg);
typedef void* (*CoStackMemAlloc)(size_t uSize);
typedef void (*CoStackMemFree)(void * pMem);

struct coroutine;


struct schedule
{
    ucontext_t ctx;              // libc��context����
    bool isDynamicStack;    // �Ƿ�����ʱ��̬����coroutine��ջ�ڴ�
    coroutine * pRunCo;          // �������е�coroutine
    CoStackMemAlloc allocfunc;   // ���ڶ�̬ջģʽ����̬ջ�ռ���亯��
    CoStackMemFree freefunc;     // ���ڶ�̬ջģʽ����̬ջ�ռ���պ���
    int secondstackmaxsize;    // ���ڶ�̬ջģʽ����¼��̬ջ����
    char * secondstack;        // ���ڶ�̬ջģʽ����¼��̬ջ�ռ�
    int firststackmaxsize;    // ���ڶ�̬ջģʽ����¼��̬ջ����
    char * firststack;        // ���ڶ�̬ջģʽ����¼��̬ջ�ռ�
};

// ����Э�̽ṹ
struct coroutine 
{
	coroutine_func func; // ���ú���ָ��
	void* arg;           // ���ú����Ĳ���ָ��
	ucontext_t ctx;      // libc��context����
	int status;          // Э�̵�״̬
    coroutine * parent;  // Э�̵ĸ�Э�̣���yield��ʱ��ָ�����Э�̵�ִ��
    int stacksize;       // ���ڶ�̬ջģʽ����¼��̬ջ�ĵ�ǰ��С
    int stackmaxsize;    // ���ڶ�̬ջģʽ����¼��̬ջ����
    char * stack;        // ���ڶ�̬ջģʽ����¼��̬ջ�ռ�
    bool usesecondstack;
};


/***
    *  @brief   Э�̳�ʼ������
    *  @param   pCo: ��Ҫ��ʼ����Э�̽ṹ��ָ�룬�ⲿ���𴴽�
    *  @param   func: pCo�󶨵ĺ���
    *  @param   arg: �󶨵ĺ����Ĳ���
    *  @param   pStack: ��Э�̵�ջ�ռ��ַ
    *  @param   iStackSize: ��Э�̵�ջ�ռ��С
    *  @param   sch: ��Э�̵ĵ�����
    *  @return  ����0: �ɹ�
                С��0: ʧ��
***/
int co_init(coroutine* pCo, coroutine_func func, void* arg, char * pStack, size_t iStackSize, schedule*  sch);

/***
    *  @brief   Э�ָ̻�����
    *  @param   pCo: ��Ҫ�ָ���Э�̽ṹ��ָ��(һ����schedule�������)
    *  @param   sch: ��Э�̵ĵ�����
    *  @return  ����0: �ɹ�
                С��0: ʧ��
***/
int co_resume(coroutine* pCo, schedule*  sch);

/***
    *  @brief   Э�̹�����
    *  @param   sch: ��Э�̵ĵ�����
    *  @return  ����0: �ɹ�
                С��0: ʧ��
***/
int co_yield(schedule*  sch);


int co_runningstacksize(schedule*  sch);

#endif
