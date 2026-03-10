#include <stdio.h>
#include <stdlib.h>
#include "tree.h"
#include "piece.h"

#define SIZE 190

Tree* tree_create(void* elt)
{
	Tree* r = malloc(sizeof(Tree));
	if (r != NULL)
	{
		r->elt = elt;
		r->left = NULL;
		r->right = NULL;
		r->prev = NULL;
		r->height = 0;
	}
	return r;
}

Tree* tree_helper(Tree* t, void* elt, int (*cmp)(Tree*, void*))
{
	if (t == NULL)
	{
		return NULL;
	}

	int delta = (*cmp)(t, elt);
	if (delta == 1)
	{
		return tree_helper(t->right, elt, cmp);
	}
	else if (delta == -1)
	{
		return tree_helper(t->left, elt, cmp);
	}
	else
	{
		return t;
	}
}

Tree* tree_add_elt(Tree* t, void* elt, int (*cmp)(Tree*, void*), void (*update_relative_info)(Tree*))
{
	if (t == NULL)
	{
		return tree_create(elt);
	}

	int delta = (*cmp)(t, elt);
	if (delta == 1)
	{
		if (t->right == NULL)
		{
			t->right = tree_create(elt);
			t->right->prev = t;
			(*update_relative_info)(t->right);
			return tree_balance(t, cmp, update_relative_info);
		}
		return tree_add_elt(t->right, elt, cmp, update_relative_info);
	}
	else if (delta == -1)
	{
		if (t->left == NULL)
		{
			t->left = tree_create(elt);
			t->left->prev = t;
			return tree_balance(t, cmp, update_relative_info);
		}
		return tree_add_elt(t->left, elt, cmp, update_relative_info);
	}
	else
	{
		return NULL;
	}
}

Tree* tree_rm(Tree* t, void* elt, int (*cmp)(Tree*, void*), void (*free_node)(void*), void (*update_relative_info)(Tree*))
{
	t = tree_helper(t, elt, cmp);
	
	if (t != NULL)
	{
		Tree** spot_to_fill;
		if (t->prev == NULL)
		{
			spot_to_fill = NULL;
		}
		else if (t->prev->right == t)
		{
			spot_to_fill = &t->prev->right;
		}
		else
		{
			spot_to_fill = &t->prev->left;
		}

		(*free_node)(t->elt);
		if (t->left == NULL && t->right == NULL)
		{
			if (spot_to_fill == NULL)
			{
				free(t);
				return NULL;
			}
			Tree* store;
			*spot_to_fill = NULL;
			store = t->prev;
			free(t);
			return tree_balance(store, cmp, update_relative_info);
		}
		else if (t->left == NULL)
		{
			Tree* store;
			if (spot_to_fill != NULL)
			{
				*spot_to_fill = t->right;
				store = t->prev;
			}
			else
			{
				store = t->right;
			}
			t->right->prev = t->prev;
			free(t);
			return tree_balance(store, cmp, update_relative_info);
		}
		else if (t->right == NULL)
		{
			Tree* store;
			if (spot_to_fill != NULL)
			{
				*spot_to_fill = t->left;
				store = t->prev;
			}
			else
			{
				store = t->left;
			}
			t->left->prev = t->prev;
			free(t);
			return tree_balance(store, cmp, update_relative_info);
		}
		else
		{
			if (t->left->height >= t->right->height)
			{
				Tree* store = t->left->right;
				Tree* store2 = t->right;
				if (spot_to_fill != NULL)
				{
					*spot_to_fill = t->left;
				}
				t->left->right = t->right;
				t->right->prev = t->left;
				t->left->prev = t->prev;
				free(t);
				if (store == NULL)
				{
					return tree_balance(store2->prev, cmp, update_relative_info);
				}
				else
				{
					Tree* i = store2;
					while (1)
					{
						if (i->left == NULL)
						{
							break;
						}
						i = i->left;
					}
					i->left = store;
					store->prev = i;
					return tree_balance(i, cmp, update_relative_info);
				}
			}
			else
			{
				Tree* store = t->right->left;
				Tree* store2 = t->left;
				if (spot_to_fill != NULL)
				{
					*spot_to_fill = t->right;
				}
				t->right->left = t->left;
				t->left->prev = t->right;
				t->right->prev = t->prev;
				free(t);
				if (store == NULL)
				{
					return tree_balance(store2->prev, cmp, update_relative_info);
				}
				else
				{
					Tree* i = store2;
					while (1)
					{
						if (i->right == NULL)
						{
							break;
						}
						i = i->right;
					}
					i->right = store;
					store->prev = i;
					return tree_balance(i, cmp, update_relative_info);
				}
			}
		}
	}

	return NULL;
}

Tree* tree_add_tree(Tree* t, Tree* to_add, int (*cmp)(Tree*, void*), void (*update_relative_info)(Tree*), bool balance)
{
	if (t == NULL)
	{
		return NULL;
	}
	if (to_add == NULL)
	{
		return t;
	}

	int delta = (*cmp)(t, to_add->elt);
	if (delta == 1)
	{
		if (t->right == NULL)
		{
			t->right = to_add;
			to_add->prev = t;
			if (balance)
			{
				return tree_balance(t, cmp, update_relative_info);
			}
			else
			{
				return NULL;
			}
		}
		return tree_add_tree(t->right, to_add, cmp, update_relative_info, balance);
	}
	else if (delta == -1)
	{
		if (t->left == NULL)
		{
			t->left = to_add;
			to_add->prev = t;
			if (balance)
			{
				return tree_balance(t, cmp, update_relative_info);
			}
			else
			{
				return NULL;
			}
		}
		return tree_add_tree(t->left, to_add, cmp, update_relative_info, balance);
	}
	else
	{
		return NULL;
	}
}

void* tree_get(Tree* t, void* elt, int (*cmp)(Tree*, void*))
{
	Tree* r = tree_helper(t, elt, cmp);
	if (r == NULL)
	{
		return NULL;
	}
	return r->elt;
}

void tree_free(Tree* t, void (*free_node)(void*))
{
	if (t == NULL)
	{
		return;
	}
	(*free_node)(t->elt);
	Tree* storel = t->left;
	Tree* storer = t->right;
	free(t);
	tree_free(storel, free_node);
	tree_free(storer, free_node);
}

Tree* tree_balance(Tree* t, int (*cmp)(Tree*, void*), void (*update_relative_info)(Tree*))
{
	if (t == NULL)
	{
		return NULL;
	}
	int left_height;
	int right_height;
	if (t->left == NULL)
	{
		left_height = -1;
	}
	else
	{
		left_height = t->left->height;
	}
	if (t->right == NULL)
	{
		right_height = -1;
	}
	else
	{
		right_height = t->right->height;
	}

	if (left_height - right_height <= 1 && left_height - right_height >= -1)
	{
		update_height(t, update_relative_info);
		if (t->prev == NULL)
		{
			return t;
		}
		return tree_balance(t->prev, cmp, update_relative_info);
	}
	else
	{
		if (right_height > left_height)
		{
			int left_height;
			int right_height;
			if (t->right->left == NULL)
			{
				left_height = -1;
			}
			else
			{
				left_height = t->right->left->height;
			}
			if (t->right->right == NULL)
			{
				right_height = -1;
			}
			else
			{
				right_height = t->right->right->height;
			}

			if (left_height > right_height)
			{
				tree_rotate(t->right, update_relative_info);
			}
			t = tree_rotate(t, update_relative_info);
			if (t->prev == NULL)
			{
				return t;
			}
			else
			{
				return tree_balance(t->prev, cmp, update_relative_info);
			}
		}
		else
		{
			int left_height;
			int right_height;
			if (t->left->left == NULL)
			{
				left_height = -1;
			}
			else
			{
				left_height = t->left->left->height;
			}
			if (t->left->right == NULL)
			{
				right_height = -1;
			}
			else
			{
				right_height = t->left->right->height;
			}

			if (right_height > left_height)
			{
				tree_rotate(t->left, update_relative_info);
			}
			t = tree_rotate(t, update_relative_info);
			if (t->prev == NULL)
			{
				return t;
			}
			else
			{
				return tree_balance(t->prev, cmp, update_relative_info);
			}
		}
	}
}

Tree* tree_rotate(Tree* t, void (*update_relative_info)(Tree*))
{
	if (t == NULL)
	{
		return NULL;
	}
	int left_height;
	int right_height;
	if (t->left == NULL)
	{
		left_height = -1;
	}
	else
	{
		left_height = t->left->height;
	}
	if (t->right == NULL)
	{
		right_height = -1;
	}
	else
	{
		right_height = t->right->height;
	}

	if (left_height >= right_height)
	{
		if (t->prev != NULL)
		{
			if (t->prev->right == t)
			{
				t->prev->right = t->left;
			}
			else
			{
				t->prev->left = t->left;
			}
		}

		Tree* store = t->left->right;
		Tree* store2 = t->left;
		t->left->right = t;
		t->left->prev = t->prev;
		t->prev = t->left;
		t->left = store;
		if (store != NULL)
		{
			store->prev = t;
		}
		update_height(t, update_relative_info);
		update_height(store2, update_relative_info);
		return store2;
	}
	else
	{
		if (t->prev != NULL)
		{
			if (t->prev->left == t)
			{
				t->prev->left = t->right;
			}
			else
			{
				t->prev->right = t->right;
			}
		}

		Tree* store = t->right->left;
		Tree* store2 = t->right;
		t->right->left = t;
		t->right->prev = t->prev;
		t->prev = t->right;
		t->right = store;
		if (store != NULL)
		{
			store->prev = t;
		}
		update_height(t, update_relative_info);
		update_height(store2, update_relative_info);
		return store2;
	}
}

void update_height(Tree* t, void (*update_relative_info)(Tree*))
{
	if (t == NULL)
	{
		return;
	}

	if (update_relative_info != NULL)
	{
		(*update_relative_info)(t);
	}
	if (t->right == NULL && t->left == NULL)
	{
		t->height = 0;
	}
	else if (t->left == NULL)
	{
		t->height = t->right->height + 1;
	}
	else if (t->right == NULL)
	{
		t->height = t->left->height + 1;
	}
	else
	{
		if (t->right->height > t->left->height)
		{
			t->height = t->right->height + 1;
		}
		else
		{
			t->height = t->left->height + 1;
		}
	}
}

void print_info(Tree* t, void (*print_elt)(void*))
{
	if (t == NULL)
	{
		return;
	}
	fprintf(stderr, "elt: (");
	if (print_elt != NULL)
	{
		(*print_elt)(t->elt);
	}
	fprintf(stderr, ") | ptr: %p | left: %p | right: %p | prev: %p\n", t, t->left, t->right, t->prev);
	print_info(t->left, print_elt);
	print_info(t->right, print_elt);
	fprintf(stderr, "\n");
}

void print_tree(Tree* t, bool reset, int row, int col)
{
	static char lines[SIZE][SIZE];
	static int max_row = 0;
	if (reset)
	{
		for (int i = 0; i < SIZE; i++)
		{
			for (int j = 0; j < SIZE; j++)
			{
				lines[i][j] = ' ';
			}
		}
	}
	if (t == NULL)
	{
		return;
	}
	if (row > max_row)
	{
		max_row = row;
	}
	lines[row][col + (SIZE / 2)] = (*(int*) t->elt) + 'a';
	print_tree(t->left, false, row + 1, col - (SIZE / (5 * (row + 1))));
	print_tree(t->right, false, row + 1, col + (SIZE / (5 * (row + 1))));
	if (row == 0)
	{
		for (int i = 0; i <= max_row; i++)
		{
			lines[i][SIZE - 1] = '\0';
			printf("row %d: %s\n", i, lines[i]);
		}
		//print_info(t);
	}
}

void recursive_update_to_root(Tree* t, void (*update_relative_info)(Tree*))
{
	if (t == NULL)
	{
		return;
	}

	(*update_relative_info)(t);
	recursive_update_to_root(t->prev, update_relative_info);
}

bool tree_find(Tree* t, Tree* to_find)
{
	if (t == NULL)
	{
		return false;
	}

	if (t == to_find)
	{
		return true;
	}
	return tree_find(t->left, to_find) || tree_find(t->right, to_find);
}
