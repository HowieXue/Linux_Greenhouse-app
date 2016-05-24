
//=============================================================
// �ļ����ƣ�lnklst.h
// ����������ͨ������������

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

/* ��ʼ������ */
INLINE void List_Init(PLISTHEAD list)				{ list->head = NULL; list->current = NULL; }

/* ��ȡָ���ڵ�,�ú����������List_GetNext,���λ�ȡ��n��ʼ�Ľڵ� */
void *List_GetFrom(PLISTHEAD list, void *n);

/* ��ȡ�׽ڵ�,�ú����������List_GetNext,���λ�ȡ�����еĽڵ� */
INLINE void *List_GetFirst(PLISTHEAD list)			{ return List_GetFrom(list, NULL); }

/* ��ȡβ�ڵ�,���øú�����,List_GetNext���������������ҽڵ� */
void *List_GetLast(PLISTHEAD list);

/* ��ȡ��һ���ڵ�,��ʼ��ȡλ����List_GetFrom, List_GetFirst��List_GetLastָ�� */
void *List_GetNext(PLISTHEAD list);

/* ������ָ��λ�ò���ڵ� */
int List_Insert(PLISTHEAD list, void *node, void *to, NOD_POSITION before_or_after);

/* ������ͷ����ڵ� */
INLINE int List_AddToHead(PLISTHEAD list, void *n)	{ return List_Insert(list, n, NULL, NOD_BEFORE); }

/* ������β����ڵ� */
INLINE int List_AddToTail(PLISTHEAD list, void *n)	{ return List_Insert(list, n, NULL, NOD_AFTER); }

/* ��������ɾ���ڵ� */
void List_Delete(PLISTHEAD list, void *n);

/* ������Ŵ�������ɾ���ڵ�*/
void List_DeleteAt(PLISTHEAD list, int nID);

/* ������ͷ����ڵ� */
INLINE int List_Push(PLISTHEAD list, void *n) { return List_Insert(list, n, NULL, NOD_BEFORE); }

/* ������β����ڵ� */
INLINE int List_Appand(PLISTHEAD list, void *n) { return List_Insert(list, n, NULL, NOD_AFTER); }

/* �õ�����ͷ�ڵ� */
INLINE void *List_Pop(PLISTHEAD list) { void *ret = list->head; List_Delete(list, ret); return ret; }

/* �ƶ�moved�ڵ㵽to�ڵ��ǰ���ߺ� */
int List_Move(PLISTHEAD list, void *moved, void *to, NOD_POSITION before_or_after);

/* �������������,���򷽷���list->cmpָ�� */
void List_Sort(PLISTHEAD list);

/* ������ʽ���ڵ�������� */
int List_OrderedAdd(PLISTHEAD list, void *nod);

/* ��ȡ�����нڵ���Ŀ */
int List_GetCount(PLISTHEAD list);

/* ������Ż�ȡ�ڵ� */
void *List_GetAt(PLISTHEAD list, int nID);


#ifdef __cplusplus
};
#endif

#endif//_LNKLST_H_
