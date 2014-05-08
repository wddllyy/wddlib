#ifndef _OBJ_POOL_H_
#define _OBJ_POOL_H_
// free chain����Ĺ̶���С��
// ����
// 1 get alloc del��ʱ�临�Ӷȶ�ΪO(1)
// 2 alloc��id �̶��������ʺ���������Ҫ��϶�����罻��
// 3 ÿ��node����ռ�ά������16�ֽ�
enum OBJPOOL_ERRORCODE
{
    SUCCESS = 0,
    ERR_OBJPOOL_NULLPTR = -1,
    ERR_OBJPOOL_MEMOVER = -2,
    ERR_OBJPOOL_FULL = -3,
    ERR_OBJPOOL_BLANK = -4,
    ERR_OBJPOOL_LOGICERROR = -5,
};


struct TPoolIdx
{
    int m_iNextIdx;
    int m_iPrevIdx;
    int m_iAccumulate;
    int m_iUsedFlag;
};
struct TObjPool
{
    int m_iUsedCount;
    int m_iFreeHead;
    int m_iFreeTail;
    int m_iUsedHead;
    int m_iCapacity;
    int m_iUnitSize;
    // ���������������ֿ�����һ����Ϊ���ڴ�ʵ��ռ�ÿ��ǣ�ȱҳ�ж�ʱ������mmap�ϴ��obj�飬һ����Ϊ�˷�ֹobjԽ��д����������Ӱ��
    TPoolIdx * m_pIdx;
    char * m_pObj;
};

typedef TObjPool* TObjPoolPtr;

#define GET_POOL_MEM_SIZE(ITEMSIZE, POOL_CAPACITY) ((ITEMSIZE + sizeof(TPoolIdx)) * POOL_CAPACITY + sizeof(TObjPool))


//************************************
// Method:    objpool_init
// FullName:  objpool_init
// Access:    public 
// Returns:   int OBJPOOL_ERRORCODE
// Qualifier: ��ʼ�������
// Parameter: TObjPool * * pObjPool ��Ҫ��ʼ����ָ�룬����ʹ��TObjPoolPtr�ṹ
// Parameter: size_t iMax ���ڵ����
// Parameter: size_t iUnit ÿ���ڵ��С
//************************************
int objpool_init(TObjPool ** pObjPool, size_t iMax, size_t iUnit);


//************************************
// Method:    objpool_destroy
// FullName:  objpool_destroy
// Access:    public 
// Returns:   int OBJPOOL_ERRORCODE
// Qualifier: ���ٶ����
// Parameter: TObjPool * * pObjPool
//************************************
int objpool_destroy(TObjPool ** pObjPool);


//************************************
// Method:    objpool_get
// FullName:  objpool_get
// Access:    public 
// Returns:   void* ��ЧΪNULL
// Qualifier: ��ȡ��ʱ�临�Ӷ�O(1)
// Parameter: TObjPool * pObjPool
// Parameter: int iIdx
//************************************
void* objpool_get( TObjPool* pObjPool, int iIdx);



//************************************
// Method:    objpool_get_bypos
// FullName:  objpool_get_bypos
// Access:    public 
// Returns:   void* ��ЧΪNULL
// Qualifier: ����λ��ƫ������ȡ�ڵ㣬��Χ[0, objpool_get_used_items()-1] ��������ʹ��
// Parameter: TObjPool * pObjPool
// Parameter: int iPos
//************************************
void* objpool_get_bypos(TObjPool* pObjPool, int iPos);


//************************************
// Method:    objpool_alloc
// FullName:  objpool_alloc
// Access:    public 
// Returns:   int OBJPOOL_ERRORCODE
// Qualifier: ����һ���ڵ�
// Parameter: TObjPool * pObjPool
//************************************
int objpool_alloc(TObjPool* pObjPool);


//************************************
// Method:    objpool_free_bypos
// FullName:  objpool_free_bypos
// Access:    public 
// Returns:   int OBJPOOL_ERRORCODE
// Qualifier: ��λ���ͷ�һ���ڵ㣬��Χ[0, objpool_get_used_items()-1]��������ʹ��
// Parameter: TObjPool * pObjPool
// Parameter: int iPos
//************************************
int objpool_free_bypos(TObjPool* pObjPool, int iPos);


//************************************
// Method:    objpool_free
// FullName:  objpool_free
// Access:    public 
// Returns:   int OBJPOOL_ERRORCODE
// Qualifier: �ͷ�һ���ڵ�
// Parameter: TObjPool * pObjPool
// Parameter: int iIdx
//************************************
int objpool_free( TObjPool* pObjPool, int iIdx );


//************************************
// Method:    objpool_get_used_items
// FullName:  objpool_get_used_items
// Access:    public 
// Returns:   size_t
// Qualifier: �ѷ����ȥ�Ľڵ����
// Parameter: TObjPool * pObjPool
//************************************
size_t objpool_get_used_items( TObjPool* pObjPool );


//************************************
// Method:    objpool_get_size
// FullName:  objpool_get_size
// Access:    public 
// Returns:   size_t
// Qualifier: ���ڵ��������
// Parameter: TObjPool * pObjPool
//************************************
size_t objpool_get_size( TObjPool *pObjPool );


enum TRAVEL_RET
{
    DO_THING = 0,
    BREAK_TRAVEL = 1, // ֹͣ����
    REMOVE_ME = 2, // ɾ���ýڵ�
};

//************************************
// Method:    OBJPOOL_TRAVEL_FUNC
// FullName:  OBJPOOL_TRAVEL_FUNC
// Access:    public 
// Returns:   TRAVEL_RET 
// Qualifier: �����ص�����
// Parameter: void* pvNode
// Parameter: void* pvArg
//************************************
typedef TRAVEL_RET (*OBJPOOL_TRAVEL_FUNC)(void* pvNode, void* pvArg);


//************************************
// Method:    objpool_travel_used
// FullName:  objpool_travel_used
// Access:    public 
// Returns:   int
// Qualifier: �������з��������Ч�ڵ�
// Parameter: TObjPool * pObjPool
// Parameter: OBJPOOL_TRAVEL_FUNC pfnTravel
// Parameter: void * pvArg
//************************************
int objpool_travel_used(TObjPool *pObjPool, OBJPOOL_TRAVEL_FUNC pfnTravel, void* pvArg);

#endif  // _OBJ_POOL_H_
