#include <stdlib.h>
#include "piece_table.h"
#include "global.h"
#include "tree.h"

Piece* make_piece(char** text, int start_index, int len, int chars_contained)
{
	Piece* r = malloc(sizeof(Piece));
	if (r != NULL)
	{
		r->text = text;
		r->start_index = start_index;
		r->len = len;
		r->chars_contained = chars_contained;
		r->lines_contained = 0;
		for (int i = start_index; i < len; i++)
		{
			if ((*text)[i] == '\n')
			{
				r->lines_contained++;
			}
		}
	}
	return r;
}

void free_piece(void* v)
{
	free(v);
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
	else if (two->chars_contained <= one->chars_contained)
	{
		if (two->chars_contained > one->chars_contained - one->len)
		{
			return 0;
		}
		return -1;
	}
	else
	{
		return 0;
	}
}

int piece_compare_lines(void* p1, void* p2)
{
	Piece* one = (Piece*) p1;
	Piece* two = (Piece*) p2;

	if (one == NULL || two == NULL)
	{
		return 0;
	}

	if (two->lines_contained > one->lines_contained)
	{
		return 1;
	}
	else if (two->lines_contained <= one->lines_contained)
	{
		if (two->lines_contained > one->lines_contained - one->len)
		{
			return 0;
		}
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
	if (p->text == NULL || *p->text == NULL)
	{
		return;
	}

	if (t->left == NULL || t->left->elt == NULL)
	{
		if (t->prev == NULL || t->prev->elt == NULL)
		{
			p->chars_contained = p->len;
			p->lines_contained = 0;
			for (int i = p->start_index; i < p->len; i++)
			{
				if ((*p->text)[i] == '\n')
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
				for (int i = p->start_index; i < p->len; i++)
				{
					if ((*p->text)[i] == '\n')
					{
						p->lines_contained++;
					}
				}
			}
			else
			{
				p->chars_contained = p->len;
				p->lines_contained = 0;
				for (int i = p->start_index; i < p->len; i++)
				{
					if ((*p->text)[i] == '\n')
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
		for (int i = p->start_index; i < p->len; i++)
		{
			if ((*p->text)[i] == '\n')
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
		Piece* p = make_piece(&r->original, 0, len, len);
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
	finder.chars_contained = index + 1;

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

	if (pt->append_len + 1 > pt->append_size)
	{
		char* new_buf = malloc(sizeof(char) * (pt->append_size + APPEND_SIZE));
		if (new_buf == NULL)
		{
			return;
		}
		for (int i = 0; i < pt->append_len; i++)
		{
			new_buf[i] = pt->append[i];
		}
		free(pt->append);
		pt->append = new_buf;
		pt->append_size += APPEND_SIZE;
	}
	if (*p->text == pt->append && p->start_index + p->len == pt->append_len && p->chars_contained == index)
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
		Piece* new_one = make_piece(p->text, p->start_index, index - p->chars_contained + p->len, index);
		if (new_one == NULL)
		{
			return;
		}
		Piece* new_two = make_piece(p->text, p->start_index + new_one->len, p->len - new_one->len, new_one->chars_contained + p->len - new_one->len + 1);
		if (new_two == NULL)
		{
			free_piece(new_one);
			return;
		}
		pt->pieces = tree_rm(pt->pieces, &finder, &piece_compare, &free_piece, &update_info);
		pt->pieces = tree_add_elt(pt->pieces, new_one, &piece_compare, &update_info);
		pt->pieces = tree_add_elt(pt->pieces, new_two, &piece_compare, &update_info);

		pt->append[pt->append_len] = c;
		pt->append_len++;

		Piece* new_piece = make_piece(&pt->append, pt->append_len, 1, index + 1);
		if (new_piece == NULL)
		{
			return;
		}
		pt->pieces = tree_add_elt(pt->pieces, new_piece, &piece_compare, &update_info);
	}
}

void pt_rm(PieceTable* pt, int index)
{
	if (pt == NULL)
	{
		return;
	}

	Piece finder;
	finder.chars_contained = index + 1;
	Piece* p = (Piece*) tree_get(pt->pieces, &finder, &piece_compare);
	if (p == NULL)
	{
		return;
	}
	if (p->text == NULL)
	{
		return;
	}
	if (*p->text == NULL)
	{
		return;
	}

	// if we are removing a character on the edge of a piece we can just adjust the piece
	// otherwise we must break it into two pieces
	if (index - p->chars_contained + p->len == 0)
	{
		if ((*p->text)[p->start_index] == '\n')
		{
			p->lines_contained--;
		}
		p->chars_contained--;
		p->len--;
		p->start_index++;
	}
	else if (index - p->chars_contained + p->len == p->len - 1)
	{
		if ((*p->text)[p->start_index + p->len - 1] == '\n')
		{
			p->lines_contained--;
		}
		p->chars_contained--;
		p->len--;
	}
	else
	{
		Piece* new_one = make_piece(p->text, p->start_index, index - p->chars_contained + p->len, index);
		if (new_one == NULL)
		{
			return;
		}
		Piece* new_two = make_piece(p->text, p->start_index + new_one->len + 1, p->len - new_one->len - 1, new_one->chars_contained + (p->len - new_one->len - 1));
		if (new_two == NULL)
		{
			free_piece(new_one);
			return;
		}
		pt->pieces = tree_rm(pt->pieces, &finder, &piece_compare, &free_piece, &update_info);
		pt->pieces = tree_add_elt(pt->pieces, new_one, &piece_compare, &update_info);
		pt->pieces = tree_add_elt(pt->pieces, new_two, &piece_compare, &update_info);
	}
}

char pt_get(PieceTable* pt, int index)
{
	if (pt == NULL)
	{
		return '\0';
	}

	Piece finder;
	finder.chars_contained = index + 1;
	Piece* p = (Piece*) tree_get(pt->pieces, &finder, &piece_compare);
	if (p == NULL)
	{
		return '\0';
	}
	return (*p->text)[p->start_index + index - p->chars_contained + p->len];
}

int pt_get_line_index(PieceTable* pt, int line_index)
{
	if (pt == NULL)
	{
		return -1;
	}
	if (pt->pieces == NULL)
	{
		return -1;
	}
	Piece finder;
	finder.lines_contained = line_index + 1;
	Piece* p = (Piece*) tree_get(pt->pieces, &finder, &piece_compare_lines);
	if (p == NULL)
	{
		return -1;
	}

	if (p->lines_contained == line_index + 1)
	{
		return p->chars_contained - p->len;
	}
	int n = p->lines_contained;
	for (int i = p->start_index; i < p->len; i++)
	{
		if ((*p->text)[i] == '\n')
		{
			n++;
			if (n == line_index + 1)
			{
				return i + p->chars_contained - p->len;
			}
		}
	}
	return -1;
}

void pt_free(PieceTable* pt)
{
	if (pt == NULL)
	{
		return;
	}
	if (pt->original != NULL)
	{
		free(pt->original);
	}
	if (pt->append != NULL)
	{
		free(pt->append);
	}
	tree_free(pt->pieces, &free_piece);
	free(pt);
}
