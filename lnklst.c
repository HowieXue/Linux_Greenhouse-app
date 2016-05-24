
//=============================================================
// 文件名称：lnklst.c
// 功能描述：通用链表管理程序


//=============================================================
#include <stdlib.h>
#include "lnklst.h"

typedef struct basic_nod_t {
	struct basic_nod_t *next;
} BASICNOD, *PBASICNOD;

// nID 返回的是nod节点的序号,小于0表示nod不属于当前链表
static void *List_PrevNode(PLISTHEAD list, PBASICNOD nod, int *nID)
{
	int id = -1;
	PBASICNOD n = list->head;
	if(n)
	{
		id = 0;
		if(nod != n)
		{
			while(n)
			{
				id++;
				if(n->next == nod)
					break;
				n = n->next;
			}
		}
		else
			n = NULL;
	}
	if(nID != NULL)
		*nID = id;
	return n;
}

void *List_GetFrom(PLISTHEAD list, void *n)
{
	if(n == NULL)
		list->current = list->head;
	else if((list->head != n) && (List_PrevNode(list, n, NULL) == NULL))	// check if n is in the list
		return NULL;
	else
		list->current = n;
	return list->current;
}

void *List_GetLast(PLISTHEAD list)
{
	return (list->current = List_PrevNode(list, NULL, NULL));
}

void *List_GetNext(PLISTHEAD list)
{
	if(list->current != NULL)
		list->current = ((PBASICNOD)(list->current))->next;
	return list->current;
}

int List_Insert(PLISTHEAD list, void *node, void *to, NOD_POSITION before_or_after)
{
	int id = 0;
	PBASICNOD n = (PBASICNOD)node;
	PBASICNOD tt = (PBASICNOD)to;
	if(node == NULL)
		return -1;
	if(list->head == NULL)
	{
        // the link list is empty
		list->head = n;
		n->next = NULL;
	}
	else if(before_or_after == NOD_BEFORE)
	{
		// add the 'node' to the place before 'to'
		// if 'to' is NULL, then add the 'node' to the head
		if(tt == NULL)
			tt = list->head;
		if(list->head == tt)
			list->head = n;
		else
		{
			PBASICNOD prev_nod = List_PrevNode(list, tt, &id);
			if(prev_nod)
				prev_nod->next = n;
		}
		n->next = tt;
	}
	else if(before_or_after == NOD_AFTER)
	{
		// add the 'node' to the place after 'to'
		if(tt == NULL)
			tt = List_PrevNode(list, NULL, &id);
		else
		{
			List_PrevNode(list, tt, &id);		// check if tt is in the list
			if(id >= 0) id++;
		}
		if(id >= 0)
		{
			n->next = tt->next;
			tt->next = n;
//			id++;
		}
	}
	return id;
}

void List_Delete(PLISTHEAD list, void *n)
{
	PBASICNOD tmp;
	if(list->head == NULL)
		return;
	if(n == NULL)
		return;
	if(n == list->head)
		list->head = ((PBASICNOD)list->head)->next;
	tmp = List_PrevNode(list, n, NULL);
	if(tmp)
		tmp->next = ((PBASICNOD)n)->next;
}

void List_DeleteAt(PLISTHEAD list, int nID)
{
	PBASICNOD prev = NULL;
	PBASICNOD tmp = list->head;
	while(tmp && (nID > 0))
	{
		nID--;
		prev = tmp;
		tmp = tmp->next;
	}
	if(tmp && (nID == 0))		// finded
	{
		if(prev == NULL)
			list->head = tmp->next;
		else
			prev->next = tmp->next;
	}
}

int List_Move(PLISTHEAD list, void *moved, void *to, NOD_POSITION before_or_after)
{
	if((list->head == NULL) || (moved == NULL))
		return -1;
	if(moved == to)
	{
		int id;
		List_PrevNode(list, ((PBASICNOD)moved)->next, &id);
		return id;
	}
	List_Delete(list, moved);
	((PBASICNOD)moved)->next = NULL;
	return List_Insert(list, moved, to, before_or_after);
}

void List_Sort(PLISTHEAD list)
{
	PBASICNOD n1;
	PBASICNOD n2;
	int swaped = 1;
	if((list->head == NULL) || (list->cmp == NULL))
		return;
	// if the list has only 1 node, then exit
	if(((PBASICNOD)list->head)->next == NULL)
		return;
	while(swaped)
	{
		n1 = list->head;
		n2 = n1->next;
		swaped = 0;
		while(1)
		{
			if((*list->cmp)(n1, n2) > 0)
			{
				List_Move(list, n1, n2, NOD_AFTER);
				n2 = n1->next;
				swaped = 1;
			}
			else
			{
				n1 = n2;
				n2 = n2->next;
			}
			if(n2 == NULL)
				break;
		}
	}
}

int List_OrderedAdd(PLISTHEAD list, void *nod)
{
	int id = -1;
	if((list->head == NULL) || (list->cmp == NULL))
		id = List_AddToTail(list, nod);
	else
	{
		PBASICNOD n = (PBASICNOD)nod;
		PBASICNOD tmp = list->head;
		PBASICNOD bak = NULL;
		n->next = NULL;
		if((*list->cmp)(n, list->head) < 0)
		{
			return List_AddToHead(list, n);
		}
		while(tmp)
		{
			id++;
			if((*list->cmp)(n, tmp) < 0)
			{
				return List_Insert(list, nod, tmp, NOD_BEFORE);
			}
			else
			{
				bak = tmp;
				tmp = tmp->next;
			}
		}
		if(bak)
			bak->next = nod;
	}
	return id;
}

int List_GetCount(PLISTHEAD list)
{
	int count = 0;
	List_PrevNode(list, NULL, &count);
	return count+1;
}

void *List_GetAt(PLISTHEAD list, int nID)
{
	PBASICNOD tmp = list->head;
	while(tmp && (nID-- > 0))
		tmp = tmp->next;
	return tmp;
}
