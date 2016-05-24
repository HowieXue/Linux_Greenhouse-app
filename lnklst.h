
//=============================================================
// 文件名称：lnklst.h
// 功能描述：通用链表管理程序

//=============================================================
#ifndef _LNKLST_H_
#define _LNKLST_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
#define INLINE static __inline
#else
#define INLINE static inline
#endif

typedef struct {
	void *head;
	void *current;
	int (*cmp)(void *n1, void *n2);
} LISTHEAD, *PLISTHEAD;

#define LISTHEAD_INITIALIZER	{ NULL, NULL, NULL }

typedef enum {
	NOD_BEFORE = 0,
	NOD_AFTER = 1
} NOD_POSITION;

/* 初始化链表 */
INLINE void List_Init(PLISTHEAD list)				{ list->head = NULL; list->current = NULL; }

/* 获取指定节点,该函数可以配合List_GetNext,依次获取从n开始的节点 */
void *List_GetFrom(PLISTHEAD list, void *n);

/* 获取首节点,该函数可以配合List_GetNext,依次获取链表中的节点 */
INLINE void *List_GetFirst(PLISTHEAD list)			{ return List_GetFrom(list, NULL); }

/* 获取尾节点,调用该函数后,List_GetNext函数将不再向后查找节点 */
void *List_GetLast(PLISTHEAD list);

/* 获取下一个节点,起始获取位置由List_GetFrom, List_GetFirst或List_GetLast指定 */
void *List_GetNext(PLISTHEAD list);

/* 向链表指定位置插入节点 */
int List_Insert(PLISTHEAD list, void *node, void *to, NOD_POSITION before_or_after);

/* 向链表头插入节点 */
INLINE int List_AddToHead(PLISTHEAD list, void *n)	{ return List_Insert(list, n, NULL, NOD_BEFORE); }

/* 向链表尾插入节点 */
INLINE int List_AddToTail(PLISTHEAD list, void *n)	{ return List_Insert(list, n, NULL, NOD_AFTER); }

/* 从链表中删除节点 */
void List_Delete(PLISTHEAD list, void *n);

/* 根据序号从链表中删除节点*/
void List_DeleteAt(PLISTHEAD list, int nID);

/* 向链表头插入节点 */
INLINE int List_Push(PLISTHEAD list, void *n) { return List_Insert(list, n, NULL, NOD_BEFORE); }

/* 向链表尾插入节点 */
INLINE int List_Appand(PLISTHEAD list, void *n) { return List_Insert(list, n, NULL, NOD_AFTER); }

/* 拿掉链表头节点 */
INLINE void *List_Pop(PLISTHEAD list) { void *ret = list->head; List_Delete(list, ret); return ret; }

/* 移动moved节点到to节点的前或者后 */
int List_Move(PLISTHEAD list, void *moved, void *to, NOD_POSITION before_or_after);

/* 对链表进行排序,排序方法由list->cmp指定 */
void List_Sort(PLISTHEAD list);

/* 以有序方式将节点插入链表 */
int List_OrderedAdd(PLISTHEAD list, void *nod);

/* 获取链表中节点数目 */
int List_GetCount(PLISTHEAD list);

/* 根据序号获取节点 */
void *List_GetAt(PLISTHEAD list, int nID);


#ifdef __cplusplus
};
#endif

#endif//_LNKLST_H_
