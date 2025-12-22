#include <stdlib.h>
#include <math.h>
#include "LL.h"

static Node* helper(LL* lst, int index)
{
	Node* r;

	int i = 0;
	r = lst->first;
	if (abs(index - recent_index) < abs(index - i))
	{
		i = recent_index;
		r = lst->recent;
	}
	if (abs(index - (size - 1)) < abs(index - i))
	{
		i = size - 1;
		r = lst->last;
	}

	int incrementer = 1;
	if (index - i < 0)
	{
		incrementer = -1;
	}

	for (; i != index; i += incrementer)
	{
		if (incrementer == 1)
		{
			r = r->next;
		}
		else
		{
			r = r->prev;
		}
	}

	return r;
}

Node* get(LL* lst, int index)
{
	return helper(lst, index);
}

void add(LL* lst, void* elt, int index)
{
	Node* new = malloc(sizeof(Node));
	new->elt = elt;

	if (index == lst->size)
	{
		if (index == 0)
		{
			new->prev = NULL;
			new->next = NULL;
			lst->first = new;
			lst->last = new;
		}
		else
		{
			new->next = NULL;
			new->prev = lst->last;
			lst->last->next = new;
			lst->last = new;
		}
	}
	else
	{
		Node* n = helper(lst, index);
		new->next = n;
		new->prev = n->prev;
		n->prev = new;
	}
	lst->recent_index = index;
	lst->recent = new;
	lst->size++;
}

void* remove(LL* lst, int index)
{
	Node* n = helper(lst, index);
	if (n->prev != NULL)
	{
		n->prev->next = n->next;
	}
	if (n->next != NULL)
	{
		n->next->prev = n->prev;
	}
	lst->size--;
	return n;
}

LL* make_list(void)
{
	LL* r = malloc(sizeof(LL));
	r->size = 0;
	return r;
}

void free_list(LL* lst)
{
	Node* ptr = lst->first;
	for (int i = 0; i < lst->size - 1; i++)
	{
		ptr = ptr->next;
		free(ptr->last);
	}
	free(ptr);
	free(lst);
}
