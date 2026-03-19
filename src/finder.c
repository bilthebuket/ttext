#include <stdlib.h>
#include "finder.h"

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
		finder_update(r, pt);
	}
	return r;
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
		jump_table[f->looking_for[i]] = len - i - 1;
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
		for (i = 0; c != f->looking_for[i] && i < len; i++)
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
				ff->indices_found = tree_create(new);
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
				f->indices_found = tree_add_elt(f->indices_found, &new_one, &finder_node_compare, &finder_update_info);
				f->indices_found = tree_add_elt(f->indices_found, &new_two, &finder_node_compare, &finder_update_info);
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
	ff.contained = index + 1;
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

	int y = pt_get_line_index_inverse(t->pt, ff.global_char_index - fn->len + 1);
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
	t->x = ff.global_char_index - fn->len + 1 - x;
}
