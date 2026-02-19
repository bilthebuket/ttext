#include <stdlib.h>
#include "tree.h"

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

Tree* tree_helper(Tree* t, void* elt, int (*cmp)(void*, void*))
{
	if (t == NULL)
	{
		return NULL;
	}

	int delta = (*cmp)(t->elt, elt);
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

Tree* tree_add_elt(Tree* t, void* elt, int (*cmp)(void*, void*))
{
	if (t == NULL)
	{
		return NULL;
	}

	int delta = (*cmp)(t->elt, elt);
	if (delta == 1)
	{
		if (t->right == NULL)
		{
			t->right = tree_create(elt);
			t->right->prev = t;
			return tree_balance(t, cmp);
		}
		return tree_add_elt(t->right, elt, cmp);
	}
	else if (delta == -1)
	{
		if (t->left == NULL)
		{
			t->left = tree_create(elt);
			t->left->prev = t;
			return tree_balance(t, cmp);
		}
		return tree_add_elt(t->left, elt, cmp);
	}
	else
	{
		return NULL;
	}
}

Tree* tree_rm(Tree* t, void* elt, int (*cmp)(void*, void*), void (*free_node)(void*))
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
			Tree* store = t->prev;
			if (spot_to_fill != NULL)
			{
				*spot_to_fill = NULL;
			}
			free(t);
			return tree_balance(store, cmp);
		}
		else if (t->left == NULL)
		{
			if (spot_to_fill != NULL)
			{
				*spot_to_fill = t->right;
			}
			free(t);
			return tree_balance(*spot_to_fill, cmp);
		}
		else if (t->right == NULL)
		{
			if (spot_to_fill != NULL)
			{
				*spot_to_fill = t->left;
			}
			free(t);
			return tree_balance(*spot_to_fill, cmp);
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
				free(t);
				return tree_add_tree(store2, store, cmp);
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
				t->left->prev = t->left;
				free(t);
				return tree_add_tree(store2, store, cmp);
			}
		}
	}

	return NULL;
}

Tree* tree_add_tree(Tree* t, Tree* to_add, int (*cmp)(void*, void*))
{
	if (t == NULL)
	{
		return NULL;
	}
	if (to_add == NULL)
	{
		return t;
	}

	int delta = (*cmp)(t->elt, to_add->elt);
	if (delta == 1)
	{
		if (t->right == NULL)
		{
			t->right = to_add;
			return tree_balance(t, cmp);
		}
		return tree_add_tree(t->right, to_add, cmp);
	}
	else if (delta == -1)
	{
		if (t->left == NULL)
		{
			t->left = to_add;
			return tree_balance(t, cmp);
		}
		return tree_add_tree(t->left, to_add, cmp);
	}
	else
	{
		return NULL;
	}
}

void* tree_get(Tree* t, void* elt, int (*cmp)(void*, void*))
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
	(*free_node)(t->elt);
	if (t->prev != NULL)
	{
		free(t->prev);
	}
	if (t->right == NULL && t->left == NULL)
	{
		free(t);
	}
	else
	{
		tree_free(t->left, free_node);
		tree_free(t->right, free_node);
	}
}

Tree* tree_balance(Tree* t, int (*cmp)(void*, void*))
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
		if (right_height > left_height)
		{
			t->height = right_height + 1;
		}
		else
		{
			t->height = left_height + 1;
		}
		if (t->prev == NULL)
		{
			return t;
		}
		return tree_balance(t->prev, cmp);
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
				tree_rotate(t->right, cmp);
			}
			t = tree_rotate(t, cmp);
			if (t->prev == NULL)
			{
				return t;
			}
			else
			{
				return tree_balance(t->prev, cmp);
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
				tree_rotate(t->left, cmp);
			}
			t = tree_rotate(t, cmp);
			if (t->prev == NULL)
			{
				return t;
			}
			else
			{
				return tree_balance(t->prev, cmp);
			}
		}
	}
}

Tree* tree_rotate(Tree* t, int (*cmp)(void*, void*))
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
		t->left = NULL;
		update_height(t);
		update_height(store2);
		return tree_add_tree(store2, store, cmp);
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
		t->right = NULL;
		update_height(t);
		update_height(store2);
		return tree_add_tree(store2, store, cmp);
	}
}

void update_height(Tree* t)
{
	if (t == NULL)
	{
		return;
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
