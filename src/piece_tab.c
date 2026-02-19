#include <stdlib.h>
#include "piece_table.h"

Piece* make_piece(char* text, int start_index, int len, int chars_contained)
{
	Piece* r = malloc(sizeof(Piece));
	if (r != NULL)
	{
		r->text = text;
		r->start_index = start_index;
		r->len = len;
		r->chars_contained = chars_contained;
		r->lines_contained = 1;
		for (int i = 0; i < len; i++)
		{
			if (text[i] == '\n')
			{
				r->lines_contained++;
			}
		}
	}
	return r;
}

int piece_compare(void* p1, void* p2)
{
	Piece* one = (Piece*) p1;
	Piece* two = (Piece*) p2;

	if (one == NULL || two == NULL)
	{
		return 0;
	}

	if (two->chars_contained > one->chars_contained)
	{
		return 1;
	}
	else if (two->chars_contained < one->chars_contained)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void update_info(Tree* t)
{
	if (t == NULL)
	{
		return;
	}
	Piece* p = (Piece*) t->elt;
	if (p == NULL)
	{
		return;
	}

	if (t->left == NULL || t->left->elt == NULL)
	{
		if (t->prev == NULL || t->prev->elt == NULL)
		{
			p->chars_contained = p->len;
			p->lines_contained = 0;
			for (int i = start_index; i < len; i++)
			{
				if (p->text[i] == '\n')
				{
					p->lines_contained++;
				}
			}
		}
		else
		{
			Piece* p_above = (Piece*) t->prev->elt;
			if (t->prev->right == t)
			{
				p->chars_contained = p_above->chars_contained + p->len;
				p->lines_contained = p_above->lines_contained;
				for (int i = start_index; i < len; i++)
				{
					if (p->text[i] == '\n')
					{
						p->lines_contained++;
					}
				}
			}
			else
			{
				p->chars_contained = p->len;
				p->lines_contained = 0;
				for (int i = start_index; i < len; i++)
				{
					if (p->text[i] == '\n')
					{
						p->lines_contained++;
					}
				}
			}
		}
	}
	else
	{
		Piece* p_left = (Piece*) t->left->elt;
		p->chars_contained = p_left->chars_contained + p->len;
		p->lines_contained = p_left->lines_contained;
		for (int i = start_index; i < len; i++)
		{
			if (p->text[i] == '\n')
			{
				p->lines_contained++;
			}
		}
	}
}

PieceTable* pt_create(char* buf, int len)
{
	PieceTable* r = malloc(sizeof(PieceTable));
	if (r != NULL)
	{
		r->original = buf;
		r->append = malloc(sizeof(char) * APPEND_SIZE);
		if (r->append == NULL)
		{
			free(r);
			return NULL;
		}
		r->append_size = APPEND_SIZE;
		r->append_len = 0;
		Piece* p = make_piece(buf, 0, len - 1, len - 1);
		if (p == NULL)
		{
			free(r->append);
			free(r);
			return NULL;
		}
		r->pieces = tree_create(p);
		if (r->pieces == NULL)
		{
			free(p);
			free(r->append);
			free(r);
			return NULL;
		}
	}
	return r;
}

void pt_insert(PieceTable* pt, char c, int index)
{
	if (pt == NULL)
	{
		return;
	}
	Piece finder;
	finder.chars_contained = index

	Piece* p = (Piece*) tree_get(pt->pieces, &finder, &piece_compare);
	if (p == NULL)
	{
		// handles edge case of inserting character at end of document
		finder.chars_contained--;
		p = (Piece*) tree_get(pt->pieces, &finder, &piece_compare);
		if (p == NULL)
		{
			return;
		}
	}
		
	if (p->text == pt->append && p->start_index + p->len == pt->append_len)
	{
		pt->append[pt->append_len] = c;
		pt->append_len++;
		p->len++;
		p->chars_contained++;
		if (c == '\n')
		{
			p->lines_contained++;
		}
	}
	else
	{
		Piece* new_piece = make_piece(pt->append, pt->append_len, 1, index + 1);
		if (new_piece == NULL)
		{
			return;
		}
		pt->append[pt->append_len] = c;
		pt->append_len++;
		tree_add_elt(pt, new_piece, &piece_compare, &update_info);
	}
}
