#include <stdlib.h>
#include "finder.h"

#define NUM_CHARS 0xFF

Finder* finder_create(PieceTable* pt, char* looking_for)
{
	if (looking_for == NULL)
	{
		return NULL;
	}
	Finder* r = malloc(sizeof(Finder));
	if (r != NULL)
	{
		r->looking_for = looking_for;
		r->indices_found = NULL;
		finder_update(r, pt);
	}
	return r;
}

static void finder_update_info(Tree* t)
{
	FinderNode* fn = (FinderNode*) t->elt;
	fn->contained = fn->len;

	if (t->left != NULL && t->left->elt != NULL)
	{
		fn->contained += ((FinderNode*) t->left->elt)->contained;
	}
	if (t->right != NULL && t->right->elt != NULL)
	{
		fn->contained += ((FinderNode*) t->right->elt)->contained;
	}
}

void finder_update(Finder* f, PieceTable* pt)
{
	int len = 0;
	for (; f->looking_for[len] != '\0'; len++) {}

	int jump_table[NUM_CHARS];
	for (int i = 0; i < NUM_CHARS; i++)
	{
		jump_table[i] = len;
	}
	for (int i = 0; i < len; i++)
	{
		jump_table[(unsigned int) f->looking_for[i]] = len - i - 1;
	}

	int index_looking_at = len - 1;
	while (1)
	{
		PieceIterator pi;
		if (!pt_iterator_init(pt, &pi, index_looking_at - (len - 1)))
		{
			break;
		}

		char c = pt_iterate(&pi);
		int i;
		for (i = 0; c == f->looking_for[i] && i < len; i++)
		{
			c = pt_iterate(&pi);
		}

		if (i == len)
		{
			FinderFinder ff;
			ff.contained = index_looking_at + 1;
			ff.global_char_index = -1;

			FinderNode* fn = (FinderNode*) tree_get(f->indices_found, &ff, &finder_finder_compare);
			if (fn == NULL)
			{
				FinderNode* new = malloc(sizeof(FinderNode));
				if (new == NULL)
				{
					return;
				}
				new->contained = index_looking_at + 1;

				new->len = index_looking_at + 1;
				// if there are existing finder nodes then this node will span in between the rightmost finder node and this latest instance of what we're searching for
				// the number of characters between the end of the rightmost node and the end of the new node is total contained characters in the tree
				if (f->indices_found != NULL && f->indices_found->elt != NULL)
				{
					new->len -= ((FinderNode*) f->indices_found->elt)->contained;
				}

				f->indices_found = tree_add_elt(f->indices_found, new, &finder_node_compare, &finder_update_info);

				index_looking_at += len;
			}
			else
			{
				FinderNode* new_one = malloc(sizeof(FinderNode));
				if (new_one == NULL)
				{
					return;
				}
				FinderNode* new_two = malloc(sizeof(FinderNode));
				if (new_two == NULL)
				{
					return;
				}

				new_two->contained = ff.global_char_index + 1;
				new_two->len = ff.global_char_index + fn->len - index_looking_at;
				new_one->contained = index_looking_at + 1;
				new_one->len = fn->len - new_two->len;

				ff.contained = index_looking_at + 1;

				f->indices_found = tree_rm(f->indices_found, &ff, &finder_finder_compare, &free, &finder_update_info);
				f->indices_found = tree_add_elt(f->indices_found, new_one, &finder_node_compare, &finder_update_info);
				f->indices_found = tree_add_elt(f->indices_found, new_two, &finder_node_compare, &finder_update_info);

				index_looking_at += len;
			}
		}
		else
		{
			index_looking_at++;
			index_looking_at += jump_table[(int) pt_get(pt, index_looking_at)];
		}
	}
}

void finder_free(Finder* f)
{
	if (f != NULL)
	{
		tree_free(f->indices_found, &free);
		free(f->looking_for);
		free(f);
	}
}

void find_next(Tab* t, Finder* f)
{
	if (t == NULL || f == NULL)
	{
		return;
	}
	FinderFinder ff;
	int index = pt_get_line_index(t->pt, t->y);
	if (index == -1)
	{
		return;
	}
	index += t->x;

	// plus two because if we just found something we want to go to the next one
	ff.contained = index + 2;
	ff.global_char_index = -1;
	FinderNode* fn = (FinderNode*) tree_get(f->indices_found, &ff, &finder_finder_compare);
	
	if (fn == NULL)
	{
		ff.contained = 1;
		ff.global_char_index = -1;
		fn = (FinderNode*) tree_get(f->indices_found, &ff, &finder_finder_compare);
		if (fn == NULL)
		{
			return;
		}
	}

	int y = pt_get_line_index_inverse(t->pt, ff.global_char_index + fn->len - 1);
	if (y < 0)
	{
		return;
	}
	int x = pt_get_line_index(t->pt, y);
	if (x < 0)
	{
		return;
	}
	t->y = y;
	t->x = ff.global_char_index + fn->len - 1 - x;
}

int finder_node_compare(Tree* t, void* v)
{
	if (t == NULL)
	{
		return 0;
	}

	FinderNode* fn1 = (FinderNode*) t->elt;
	FinderNode* fn2 = (FinderNode*) v;
	if (fn1 == NULL || fn2 == NULL)
	{
		return 0;
	}

	int left_length = 0;
	if (t->left != NULL && t->left->elt != NULL)
	{
		left_length = ((FinderNode*) t->left->elt)->contained;
	}

	if (fn2->contained - fn2->len >= left_length)
	{
		if (fn2->contained - fn2->len >= left_length + fn1->len)
		{
			fn2->contained -= left_length + fn1->len;
			return 1;
		}
		return 0;
	}
	else
	{
		return -1;
	}
}

int finder_finder_compare(Tree* t, void* v)
{
	if (t == NULL)
	{
		return 0;
	}

	FinderFinder* ff = (FinderFinder*) v;
	FinderNode* fn = (FinderNode*) t->elt;
	if (fn == NULL || fn == NULL)
	{
		return 0;
	}

	if (ff->global_char_index == -1)
	{
		ff->global_char_index = fn->contained;
	}

	int left_length = 0;
	int right_length = 0;

	if (t->left != NULL && t->left->elt != NULL)
	{
		left_length = ((FinderNode*) t->left->elt)->contained;
	}
	if (t->right != NULL && t->right->elt != NULL)
	{
		right_length = ((FinderNode*) t->right->elt)->contained;
	}

	if (ff->contained <= left_length)
	{
		if (ff->contained >= 0 && (t->left == NULL || t->left->elt == NULL))
		{
			ff->global_char_index -= fn->len + right_length;
			return 0;
		}
		ff->global_char_index -= fn->len + right_length;
		return -1;
	}
	else if (ff->contained > left_length + fn->len)
	{
		ff->contained -= left_length + fn->len;
		return 1;
	}
	else
	{
		ff->global_char_index -= fn->len + right_length;
		return 0;
	}
}
