#include <stdlib.h>
#include <math.h>
#include "linked_list.h"

static Node* helper(LinkedList* lst, int index)
{
	if (lst == NULL)
	{
		return NULL;
	}

	Node* r;

	int i = 0;
	r = lst->first;
	if (abs(index - lst->recent_index) < abs(index - i))
	{
		i = lst->recent_index;
		r = lst->recent;
	}
	if (abs(index - (lst->size - 1)) < abs(index - i))
	{
		i = lst->size - 1;
		r = lst->last;
	}

	int incrementer = 1;
	if (index - i < 0)
	{
		incrementer = -1;
	}

	for (; i != index && r != NULL; i += incrementer)
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

Node* ll_get_node(LinkedList* lst, int index)
{
	if (lst->size == 0)
	{
		return NULL;
	}
	return helper(lst, index);
}

void* ll_get_elt(LinkedList* lst, int index)
{
	if (lst == NULL || lst->size == 0)
	{
		return NULL;
	}
	Node* n = ll_get_node(lst, index);
	if (n == NULL)
	{
		return NULL;
	}
	return n->elt;
}

void ll_insert(LinkedList* lst, void* elt, int index)
{
	if (lst == NULL)
	{
		return;
	}

	if (index < 0)
	{
		index = 0;
	}

	Node* new = malloc(sizeof(Node));
	if (new == NULL)
	{
		return;
	}
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
		if (n == NULL)
		{
			free(new);
			return;
		}
		new->next = n;
		new->prev = n->prev;
		if (index == 0)
		{
			lst->first = new;
		}
		else
		{
			n->prev->next = new;
		}
		n->prev = new;
	}

	lst->recent_index = index;
	lst->recent = new;
	lst->size++;
}

void* ll_rm(LinkedList* lst, int index)
{
	if (lst == NULL || lst->size < 1)
	{
		return NULL;
	}
	Node* n = helper(lst, index);
	if (n == NULL)
	{
		return NULL;
	}
	void* r = n->elt;

	if (n->prev != NULL)
	{
		n->prev->next = n->next;
	}
	else
	{
		lst->first = n->next;
	}

	if (n->next != NULL)
	{
		n->next->prev = n->prev;
		lst->recent = n->next;
	}
	else
	{
		lst->last = n->prev;
		lst->recent = n->prev;
		lst->recent_index--;
	}

	lst->size--;
	free(n);
	return r;
}

LinkedList* ll_create(void)
{
	LinkedList* r = malloc(sizeof(LinkedList));
	if (r == NULL)
	{
		return NULL;
	}
	r->size = 0;
	return r;
}

void ll_free(LinkedList* lst)
{
	if (lst == NULL)
	{
		return;
	}
	if (lst->size > 0)
	{
		Node* ptr = lst->first;
		if (ptr != NULL)
		{
			if (lst->size > 1)
			{
				for (int i = 0; i < lst->size - 1; i++)
				{
					ptr = ptr->next;
					free(ptr->prev->elt);
					free(ptr->prev);
				}
			}

			free(ptr->elt);
			free(ptr);
		}
	}
	free(lst);
}
