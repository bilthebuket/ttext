#include <stdlib.h>
#include "piece_table.h"
#include "global.h"
#include "tree.h"

void print_piece(void* v)
{
	Piece* p = (Piece*) v;
	fprintf(stderr, "str: ");
	for (int i = p->start_index; i < p->start_index + p->len; i++)
	{
		if ((*p->text)[i] == '\n')
		{
			fprintf(stderr, "\\n");
		}
		else
		{
			fprintf(stderr, "%c", (*p->text)[i]);
		}
	}
	fprintf(stderr, " | start index: %d | len: %d | chars contained: %d | lines inside: %d | lines contained: %d", p->start_index, p->len, p->chars_contained, p->lines_inside, p->lines_contained);
}
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
		r->lines_inside = 0;
		for (int i = start_index; i < start_index + len; i++)
		{
			if ((*text)[i] == '\n')
			{
				r->lines_contained++;
				r->lines_inside++;
			}
		}
	}
	return r;
}

void free_piece(void* v)
{
	free(v);
}

int piece_compare(Tree* t, void* elt)
{
	Piece* p = (Piece*) elt;
	if (t == NULL || p == NULL || t->elt == NULL)
	{
		return 0;
	}

	if (p->lines_inside == -1)
	{
		p->len = ((Piece*) t->elt)->chars_contained;
		p->lines_inside = -2;
	}

	int left_length = 0;
	int right_length = 0;
	int right_char_length = 0;

	if (t->left != NULL && t->left->elt != NULL)
	{
		left_length = ((Piece*) t->left->elt)->chars_contained;
	}
	if (t->right != NULL && t->right->elt != NULL)
	{
		right_length = ((Piece*) t->right->elt)->chars_contained;
		right_char_length = ((Piece*) t->right->elt)->chars_contained;
	}

	if (p->chars_contained <= left_length)
	{
		if (p->chars_contained >= 0 && (t->left == NULL || t->left->elt == NULL))
		{
			if (p->lines_inside == -2)
			{
				p->len -= ((Piece*) t->elt)->len + right_char_length;
			}
			return 0;
		}
		if (p->lines_inside == -2)
		{
			p->len -= ((Piece*) t->elt)->len + right_char_length;
		}
		return -1;
	}
	else if (p->chars_contained > left_length + ((Piece*) t->elt)->len)
	{
		p->chars_contained -= left_length + ((Piece*) t->elt)->len;
		return 1;
	}
	else
	{
		if (p->lines_inside == -2)
		{
			p->len -= ((Piece*) t->elt)->len + right_char_length;
		}
		return 0;
	}
}

// this is only used for pt_get_line_index
// so we can use the chars_contained field to return what index in the document
// the first character of a piece is
int piece_compare_lines(Tree* t, void* elt)
{
	Piece* p = (Piece*) elt;
	if (t == NULL || p == NULL || t->elt == NULL)
	{
		return 0;
	}

	if (p->chars_contained == -1)
	{
		p->chars_contained = ((Piece*) t->elt)->chars_contained;
		p->lines_inside = ((Piece*) t->elt)->lines_contained;
	}

	int left_length = 0;
	int right_length = 0;

	int right_char_length = 0;

	if (t->left != NULL && t->left->elt != NULL)
	{
		left_length = ((Piece*) t->left->elt)->lines_contained;
	}
	if (t->right != NULL && t->right->elt != NULL)
	{
		right_length = ((Piece*) t->right->elt)->lines_contained;
		right_char_length = ((Piece*) t->right->elt)->chars_contained;
	}

	if (p->lines_contained <= left_length)
	{
		// edge case for accessing line 0
		if (p->lines_contained >= 0 && (t->left == NULL || t->left->elt == NULL))
		{
			p->chars_contained -= ((Piece*) t->elt)->len + right_char_length;
			p->lines_inside -= ((Piece*) t->elt)->lines_inside + right_length;
			return 0;
		}
		p->chars_contained -= ((Piece*) t->elt)->len + right_char_length;
		p->lines_inside -= ((Piece*) t->elt)->lines_inside + right_length;
		return -1;
	}
	else if (p->lines_contained > left_length + ((Piece*) t->elt)->lines_inside)
	{
		p->lines_contained -= left_length + ((Piece*) t->elt)->lines_inside;
		return 1;
	}
	else
	{
		p->chars_contained -= ((Piece*) t->elt)->len + right_char_length;
		p->lines_inside -= ((Piece*) t->elt)->lines_inside + right_length;
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

	p->lines_contained = p->lines_inside;
	p->chars_contained = p->len;

	if (t->left != NULL && t->left->elt != NULL)
	{
		p->lines_contained += ((Piece*) t->left->elt)->lines_contained;
		p->chars_contained += ((Piece*) t->left->elt)->chars_contained;
	}
	if (t->right != NULL && t->right->elt != NULL)
	{
		p->lines_contained += ((Piece*) t->right->elt)->lines_contained;
		p->chars_contained += ((Piece*) t->right->elt)->chars_contained;
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
		return;
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
			p->lines_inside++;
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
	finder.lines_inside = -1;
	Tree* t = tree_helper(pt->pieces, &finder, &piece_compare);
	Piece* p = (Piece*) t->elt;
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
	if (index == finder.len)
	{
		if ((*p->text)[p->start_index] == '\n')
		{
			p->lines_contained--;
			p->lines_inside--;
		}
		p->chars_contained--;
		p->len--;
		p->start_index++;
		recursive_update_to_root(t, &update_info);
	}
	else if (index - finder.len == p->len - 1)
	{
		if ((*p->text)[p->start_index + p->len - 1] == '\n')
		{
			p->lines_contained--;
			p->lines_inside--;
		}
		p->chars_contained--;
		p->len--;
		recursive_update_to_root(t, &update_info);
	}
	else
	{
		Piece* new_one = make_piece(p->text, p->start_index, index - finder.len, index);
		if (new_one == NULL)
		{
			return;
		}
		Piece* new_two = make_piece(p->text, p->start_index + new_one->len + 1, p->len - new_one->len - 1, index + p->len - new_one->len - 1);
		if (new_two == NULL)
		{
			free_piece(new_one);
			return;
		}

		finder.chars_contained = index + 1;
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
	finder.lines_inside = -1;
	Piece* p = (Piece*) tree_get(pt->pieces, &finder, &piece_compare);
	if (p == NULL)
	{
		return '\0';
	}
	return (*p->text)[p->start_index + index - finder.len];
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
	finder.lines_contained = line_index;
	finder.chars_contained = -1;
	Piece* p = (Piece*) tree_get(pt->pieces, &finder, &piece_compare_lines);
	if (p == NULL)
	{
		return -1;
	}

	int n = 0;
	int num_to_find = line_index - finder.lines_inside;
	int i;
	for (i = p->start_index; i < p->start_index + p->len && n != num_to_find; i++)
	{
		if ((*p->text)[i] == '\n')
		{
			n++;
		}
	}
	return finder.chars_contained + i - p->start_index;
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
