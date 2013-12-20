#ifndef _OBJ_POOL_H_
#define _OBJ_POOL_H_
// free chain管理的固定大小池
// 特性
// 1 get alloc del的时间复杂度都为O(1)
// 2 alloc的id 固定增长，适合用于重入要求较多的网络交互
// 3 每个node额外空间维护开销16字节
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
    // 索引区和数据区分开来，一个是为了内存实际占用考虑，缺页中断时才真正mmap较大的obj块，一个是为了防止obj越界写对索引区的影响
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
// Qualifier: 初始化对象池
// Parameter: TObjPool * * pObjPool 需要初始化的指针，建议使用TObjPoolPtr结构
// Parameter: size_t iMax 最大节点个数
// Parameter: size_t iUnit 每个节点大小
//************************************
int objpool_init(TObjPool ** pObjPool, size_t iMax, size_t iUnit);


//************************************
// Method:    objpool_destroy
// FullName:  objpool_destroy
// Access:    public 
// Returns:   int OBJPOOL_ERRORCODE
// Qualifier: 销毁对象池
// Parameter: TObjPool * * pObjPool
//************************************
int objpool_destroy(TObjPool ** pObjPool);


//************************************
// Method:    objpool_get
// FullName:  objpool_get
// Access:    public 
// Returns:   void* 无效为NULL
// Qualifier: 获取，时间复杂度O(1)
// Parameter: TObjPool * pObjPool
// Parameter: int iIdx
//************************************
void* objpool_get( TObjPool* pObjPool, int iIdx);



//************************************
// Method:    objpool_get_bypos
// FullName:  objpool_get_bypos
// Access:    public 
// Returns:   void* 无效为NULL
// Qualifier: 利用位置偏移来获取节点，范围[0, objpool_get_used_items()-1] ，不建议使用
// Parameter: TObjPool * pObjPool
// Parameter: int iPos
//************************************
void* objpool_get_bypos(TObjPool* pObjPool, int iPos);


//************************************
// Method:    objpool_alloc
// FullName:  objpool_alloc
// Access:    public 
// Returns:   int OBJPOOL_ERRORCODE
// Qualifier: 分配一个节点
// Parameter: TObjPool * pObjPool
//************************************
int objpool_alloc(TObjPool* pObjPool);


//************************************
// Method:    objpool_free_bypos
// FullName:  objpool_free_bypos
// Access:    public 
// Returns:   int OBJPOOL_ERRORCODE
// Qualifier: 按位置释放一个节点，范围[0, objpool_get_used_items()-1]，不建议使用
// Parameter: TObjPool * pObjPool
// Parameter: int iPos
//************************************
int objpool_free_bypos(TObjPool* pObjPool, int iPos);


//************************************
// Method:    objpool_free
// FullName:  objpool_free
// Access:    public 
// Returns:   int OBJPOOL_ERRORCODE
// Qualifier: 释放一个节点
// Parameter: TObjPool * pObjPool
// Parameter: int iIdx
//************************************
int objpool_free( TObjPool* pObjPool, int iIdx );


//************************************
// Method:    objpool_get_used_items
// FullName:  objpool_get_used_items
// Access:    public 
// Returns:   size_t
// Qualifier: 已分配出去的节点个数
// Parameter: TObjPool * pObjPool
//************************************
size_t objpool_get_used_items( TObjPool* pObjPool );


//************************************
// Method:    objpool_get_size
// FullName:  objpool_get_size
// Access:    public 
// Returns:   size_t
// Qualifier: 最大节点个数容量
// Parameter: TObjPool * pObjPool
//************************************
size_t objpool_get_size( TObjPool *pObjPool );


enum TRAVEL_RET
{
    DO_THING = 0,
    BREAK_TRAVEL = 1, // 停止遍历
    REMOVE_ME = 2, // 删除该节点
};

//************************************
// Method:    OBJPOOL_TRAVEL_FUNC
// FullName:  OBJPOOL_TRAVEL_FUNC
// Access:    public 
// Returns:   TRAVEL_RET 
// Qualifier: 遍历回调函数
// Parameter: void* pvNode
// Parameter: void* pvArg
//************************************
typedef TRAVEL_RET (*OBJPOOL_TRAVEL_FUNC)(void* pvNode, void* pvArg);


//************************************
// Method:    objpool_travel_used
// FullName:  objpool_travel_used
// Access:    public 
// Returns:   int
// Qualifier: 遍历所有分配出的有效节点
// Parameter: TObjPool * pObjPool
// Parameter: OBJPOOL_TRAVEL_FUNC pfnTravel
// Parameter: void * pvArg
//************************************
int objpool_travel_used(TObjPool *pObjPool, OBJPOOL_TRAVEL_FUNC pfnTravel, void* pvArg);

#endif  // _OBJ_POOL_H_
