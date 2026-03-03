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
	if (t == NULL)
	{
		return 0;
	}

	Piece* e = (Piece*) elt;
	Piece* p = (Piece*) t->elt;
	if (p == NULL || e == NULL)
	{
		return 0;
	}

	int left_length = 0;

	if (t->left != NULL && t->left->elt != NULL)
	{
		left_length = ((Piece*) t->left->elt)->chars_contained;
	}

	if (e->chars_contained - e->len >= left_length + p->len)
	{
		e->chars_contained -= left_length + p->len;
		return 1;
	}
	else
	{
		return -1;
	}
}

int piece_finder_compare_characters(Tree* t, void* elt)
{
	if (t == NULL)
	{
		return 0;
	}

	PieceFinder* f = (PieceFinder*) elt;
	Piece* p = (Piece*) t->elt;
	if (p == NULL || f == NULL)
	{
		return 0;
	}

	if (f->global_char_index == -1)
	{
		f->global_char_index = p->chars_contained;
	}

	int left_length = 0;
	int right_length = 0;

	if (t->left != NULL && t->left->elt != NULL)
	{
		left_length = ((Piece*) t->left->elt)->chars_contained;
	}
	if (t->right != NULL && t->right->elt != NULL)
	{
		right_length = ((Piece*) t->right->elt)->chars_contained;
	}

	if (f->contained <= left_length)
	{
		if (f->contained >= 0 && (t->left == NULL || t->left->elt == NULL))
		{
			f->global_char_index -= p->len + right_length;
			return 0;
		}
		f->global_char_index -= p->len + right_length;
		return -1;
	}
	else if (f->contained > left_length + p->len)
	{
		f->contained -= left_length + p->len;
		return 1;
	}
	else
	{
		f->global_char_index -= p->len + right_length;
		return 0;
	}
}

int piece_finder_compare_lines(Tree* t, void* elt)
{
	if (t == NULL)
	{
		return 0;
	}

	PieceFinder* f = (PieceFinder*) elt;
	Piece* p = (Piece*) t->elt;
	if (p == NULL || f == NULL)
	{
		return 0;
	}

	if (f->global_char_index == -1 || f->global_line_index == -1)
	{
		f->global_char_index = p->chars_contained;
		f->global_line_index = p->lines_contained;
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

	if (f->contained <= left_length)
	{
		if (f->contained >= 0 && (t->left == NULL || t->left->elt == NULL))
		{
			f->global_char_index -= p->len + right_char_length;
			f->global_line_index -= p->lines_inside + right_length;
			return 0;
		}
		f->global_char_index -= p->len + right_char_length;
		f->global_line_index -= p->lines_inside + right_length;
		return -1;
	}
	else if (f->contained > left_length + p->lines_inside)
	{
		f->contained -= left_length + p->lines_inside;
		return 1;
	}
	else
	{
		f->global_char_index -= p->len + right_char_length;
		f->global_line_index -= p->lines_inside + right_length;
		return 0;
	}
}

void piece_update_info(Tree* t)
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
		if (buf == NULL)
		{
			r->pieces = NULL;
		}
		else
		{
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
	}
	return r;
}

void pt_insert(PieceTable* pt, char c, int index)
{
	if (pt == NULL)
	{
		return;
	}
	if (pt->pieces == NULL)
	{
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

		pt->append[pt->append_len] = c;
		Piece* new_piece = make_piece(&pt->append, pt->append_len, 1, 1);
		pt->pieces = tree_create(new_piece);
		pt->append_len++;
	}

	PieceFinder finder;
	finder.contained = index;
	finder.global_char_index = -1;

	Tree* t = tree_helper(pt->pieces, &finder, &piece_finder_compare_characters);
	if (t == NULL)
	{
		return;
	}
	Piece* p = (Piece*) t->elt;
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

	if (*p->text == pt->append && p->start_index + p->len == pt->append_len && index == finder.global_char_index + p->len)
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
		recursive_update_to_root(t, &piece_update_info);
	}
	else
	{
		Piece* new_one = make_piece(p->text, p->start_index, index - finder.global_char_index, index);
		if (new_one == NULL)
		{
			return;
		}
		Piece* new_two = make_piece(p->text, p->start_index + new_one->len, p->len - new_one->len, index + p->len - new_one->len);
		if (new_two == NULL)
		{
			free_piece(new_one);
			return;
		}

		finder.contained = index;
		pt->pieces = tree_rm(pt->pieces, &finder, &piece_finder_compare_characters, &free_piece, &piece_update_info);

		if (new_one->len > 0)
		{
			pt->pieces = tree_add_elt(pt->pieces, new_one, &piece_compare, &piece_update_info);
		}
		else
		{
			free_piece(new_one);
		}
		if (new_two->len > 0)
		{
			pt->pieces = tree_add_elt(pt->pieces, new_two, &piece_compare, &piece_update_info);
		}
		else
		{
			free_piece(new_two);
		}

		pt->append[pt->append_len] = c;
		pt->append_len++;

		Piece* new_piece = make_piece(&pt->append, pt->append_len - 1, 1, index + 1);
		if (new_piece == NULL)
		{
			return;
		}
		pt->pieces = tree_add_elt(pt->pieces, new_piece, &piece_compare, &piece_update_info);
	}

	ColorIndexFinder f;
	f.contained = index;
	f.global_char_index = -1;
	t = tree_helper(pt->color_indices, &f, &color_index_finder_compare_characters);
	if (t == NULL)
	{
		if (pt->pieces != NULL && pt->pieces->elt != NULL)
		{
			// this has to be one less than pt->pieces because we are going to add one later
			ColorIndex* ci = make_color_index(CYAN_TEXT, ((Piece*) pt->pieces->elt)->chars_contained - 1, ((Piece*) pt->pieces->elt)->chars_contained - 1);
			if (ci != NULL)
			{
				pt->color_indices = tree_create(ci);
				t = pt->color_indices;
			}
		}
	}
	if (t->elt == NULL)
	{
		return;
	}

	((ColorIndex*) t->elt)->len++;
	((ColorIndex*) t->elt)->chars_contained++;

	recursive_update_to_root(t, &color_index_update_info);
	pt_update_color_indices(pt, index);
}

void pt_rm(PieceTable* pt, int index)
{
	if (pt == NULL)
	{
		return;
	}

	PieceFinder finder;
	finder.contained = index + 1;
	finder.global_char_index = -1;
	Tree* t = tree_helper(pt->pieces, &finder, &piece_finder_compare_characters);
	if (t == NULL)
	{
		return;
	}
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
	if (index == finder.global_char_index)
	{
		if (p->len == 1)
		{
			finder.contained = index + 1;
			pt->pieces = tree_rm(pt->pieces, &finder, &piece_finder_compare_characters, &free_piece, &piece_update_info);
		}
		else
		{
			if ((*p->text)[p->start_index] == '\n')
			{
				p->lines_contained--;
				p->lines_inside--;
			}
			p->chars_contained--;
			p->len--;
			p->start_index++;
			recursive_update_to_root(t, &piece_update_info);
		}
	}
	else if (index - finder.global_char_index == p->len - 1)
	{
		if (p->start_index + p->len == pt->append_len && *p->text == pt->append)
		{
			pt->append_len--;
		}
		if (p->len == 1)
		{
			finder.contained = index + 1;
			pt->pieces = tree_rm(pt->pieces, &finder, &piece_finder_compare_characters, &free_piece, &piece_update_info);
		}
		else
		{
			if ((*p->text)[p->start_index + p->len - 1] == '\n')
			{
				p->lines_contained--;
				p->lines_inside--;
			}
			p->chars_contained--;
			p->len--;
			recursive_update_to_root(t, &piece_update_info);
		}
	}
	else
	{
		Piece* new_one = make_piece(p->text, p->start_index, index - finder.global_char_index, index);
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

		finder.contained = index + 1;
		pt->pieces = tree_rm(pt->pieces, &finder, &piece_finder_compare_characters, &free_piece, &piece_update_info);

		if (new_one->len > 0)
		{
			pt->pieces = tree_add_elt(pt->pieces, new_one, &piece_compare, &piece_update_info);
		}
		else
		{
			free_piece(new_one);
		}
		if (new_two->len > 0)
		{
			pt->pieces = tree_add_elt(pt->pieces, new_two, &piece_compare, &piece_update_info);
		}
		else
		{
			free_piece(new_two);
		}
	}

	ColorIndexFinder f;
	f.contained = index;
	f.global_char_index = -1;
	t = tree_helper(pt->color_indices, &f, &color_index_finder_compare_characters);
	if (t == NULL)
	{
		if (pt->pieces != NULL && pt->pieces->elt != NULL)
		{
			// this has to be one morethan pt->pieces because we are going to subtract one later
			ColorIndex* ci = make_color_index(CYAN_TEXT, ((Piece*) pt->pieces->elt)->chars_contained + 1, ((Piece*) pt->pieces->elt)->chars_contained + 1);
			if (ci != NULL)
			{
				pt->color_indices = tree_create(ci);
				t = pt->color_indices;
			}
		}
	}
	if (t->elt == NULL)
	{
		return;
	}

	((ColorIndex*) t->elt)->len--;
	((ColorIndex*) t->elt)->chars_contained--;

	recursive_update_to_root(t, &color_index_update_info);
	pt_update_color_indices(pt, index);
}

char pt_get(PieceTable* pt, int index)
{
	if (pt == NULL)
	{
		return '\0';
	}

	PieceFinder finder;
	finder.contained = index + 1;
	finder.global_char_index = -1;
	Piece* p = (Piece*) tree_get(pt->pieces, &finder, &piece_finder_compare_characters);
	if (p == NULL)
	{
		return '\0';
	}
	return (*p->text)[p->start_index + index - finder.global_char_index];
}

bool pt_iterator_init(PieceTable* pt, PieceIterator* pi, int index)
{
	if (pt == NULL || pi == NULL || index < 0)
	{
		return false;
	}

	PieceFinder f;
	f.contained = index + 1;
	f.global_char_index = -1;
	pi->node = tree_helper(pt->pieces, &f, &piece_finder_compare_characters);
	if (pi->node == NULL)
	{
		return false;
	}
	pi->index = index - f.global_char_index;
	return true;
}

char pt_iterate(PieceIterator* pi)
{
	if (pi == NULL || pi->node == NULL)
	{
		return '\0';
	}

	Piece* p = (Piece*) pi->node->elt;
	char c = (*p->text)[p->start_index + pi->index];
	if (pi->index == p->len - 1)
	{
		pi->index = 0;
		if (pi->node->right != NULL)
		{
			pi->node = pi->node->right;
			while (pi->node->left != NULL)
			{
				pi->node = pi->node->left;
			}
		}
		else
		{
			Tree* prev = pi->node->prev;
			while (prev != NULL && prev->right == pi->node)
			{
				pi->node = prev;
				prev = prev->prev;
			}
			pi->node = prev;
		}
	}
	else
	{
		pi->index++;
	}

	return c;
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
	PieceFinder finder;
	finder.contained = line_index;
	finder.global_char_index = -1;
	Piece* p = (Piece*) tree_get(pt->pieces, &finder, &piece_finder_compare_lines);
	if (p == NULL)
	{
		return -1;
	}

	int n = 0;
	int num_to_find = line_index - finder.global_line_index;
	int i;
	for (i = p->start_index; i < p->start_index + p->len && n != num_to_find; i++)
	{
		if ((*p->text)[i] == '\n')
		{
			n++;
		}
	}
	return finder.global_char_index + i - p->start_index;
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

int color_index_compare(Tree* t, void* elt)
{
	if (t == NULL)
	{
		return 0;
	}

	ColorIndex* e = (ColorIndex*) elt;
	ColorIndex* c = (ColorIndex*) t->elt;
	if (e == NULL || c == NULL)
	{
		return 0;
	}

	int left_length = 0;

	if (t->left != NULL && t->left->elt != NULL)
	{
		left_length = ((ColorIndex*) t->left->elt)->chars_contained;
	}

	if (e->chars_contained - e->len >= left_length + c->len)
	{
		e->chars_contained -= left_length + c->len;
		return 1;
	}
	else
	{
		return -1;
	}
}

int color_index_finder_compare_characters(Tree* t, void* elt)
{
	if (t == NULL)
	{
		return 0;
	}

	ColorIndexFinder* f = (ColorIndexFinder*) elt;
	ColorIndex* c = (ColorIndex*) t->elt;
	if (f == NULL || c == NULL)
	{
		return 0;
	}

	if (f->global_char_index == -1)
	{
		f->global_char_index = c->chars_contained;
	}

	int left_length = 0;
	int right_length = 0;

	if (t->left != NULL && t->left->elt != NULL)
	{
		left_length = ((ColorIndex*) t->left->elt)->chars_contained;
	}
	if (t->right != NULL && t->right->elt != NULL)
	{
		right_length = ((ColorIndex*) t->right->elt)->chars_contained;
	}

	if (f->contained <= left_length)
	{
		if (f->contained >= 0 && (t->left == NULL || t->left->elt == NULL))
		{
			f->global_char_index -= c->len + right_length;
			return 0;
		}
		f->global_char_index -= c->len + right_length;
		return -1;
	}
	else if (f->contained > left_length + c->len)
	{
		f->contained -= left_length + c->len;
		return 1;
	}
	else
	{
		f->global_char_index -= c->len + right_length;
		return 0;
	}
}

int pt_get_color(PieceTable* pt, int index)
{
	if (pt == NULL || pt->color_indices == NULL)
	{
		return -1;
	}

	ColorIndexFinder f;
	f.contained = index + 1;
	f.global_char_index = -1;
	ColorIndex* ci = (ColorIndex*) tree_get(pt->color_indices, &f, &color_index_finder_compare_characters);
	if (ci == NULL)
	{
		return -1;
	}
	return ci->color;
}

ColorIndex* make_color_index(int color, int len, int chars_contained)
{
	ColorIndex* r = malloc(sizeof(ColorIndex));
	if (r != NULL)
	{
		r->color = color;
		r->len = len;
		r->chars_contained = chars_contained;
	}
	return r;
}

void pt_update_color_indices(PieceTable* pt, int index)
{
	if (pt == NULL || pt->color_indices == NULL)
	{
		return;
	}

	ColorIndexFinder f;
	f.contained = index + 1;
	f.global_char_index = -1;

	Tree* t = tree_helper(pt->color_indices, &f, &color_index_finder_compare_characters);
	if (t == NULL)
	{
		return;
	}
	ColorIndex* ci = (ColorIndex*) t->elt;
	if (ci == NULL)
	{
		return;
	}

	PieceIterator pi;
	if (!pt_iterator_init(pt, &pi, f.global_char_index))
	{
		return;
	}

	// TODO: use bitflags intead of bool array

	bool chars_to_split_on[NUM_CHARS];
	// skip the first character because if a split character is at index one that means we
	// dont need to split/already split
	for (int i = 1; i < NUM_CHARS; i++)
	{
		chars_to_split_on[i] = false;
	}
	chars_to_split_on[' '] = true;
	chars_to_split_on['['] = true;
	chars_to_split_on[']'] = true;
	chars_to_split_on['{'] = true;
	chars_to_split_on['}'] = true;
	chars_to_split_on['('] = true;
	chars_to_split_on[')'] = true;

	bool split = false;

	int i;
	char c;
	for (i = 0; i < ci->len; i++)
	{
		c = pt_iterate(&pi);
		if (chars_to_split_on[(int) c])
		{
			split = true;
			break;
		}
	}

	if (split)
	{
		switch (c)
		{
			default:
			ColorIndex* new_one = make_color_index(ci->color, 1, f.global_char_index + i + 1);
			if (new_one == NULL)
			{
				return;
			}
			ColorIndex* new_two = make_color_index(ci->color, ci->len - i - 1, f.global_char_index + ci->len);
			if (new_two == NULL)
			{
				free(new_one);
				return;
			}

			ci->len -= ci->len - i;
			recursive_update_to_root(t, &color_index_update_info);

			pt->color_indices = tree_add_elt(pt->color_indices, new_one, &color_index_compare, &color_index_update_info);
			if (new_two->len > 0)
			{
				pt->color_indices = tree_add_elt(pt->color_indices, new_one, &color_index_compare, &color_index_update_info);
				pt_update_color_indices(pt, f.global_char_index);
				pt_update_color_indices(pt, f.global_char_index + i);
				pt_update_color_indices(pt, f.global_char_index + i + 1);
			}
			else
			{
				free(new_two);
				pt_update_color_indices(pt, f.global_char_index);
				pt_update_color_indices(pt, f.global_char_index + i);

			}
			break;

			case ' ':
			ColorIndex* new = make_color_index(ci->color, ci->len - i, f.global_char_index + ci->len);
			if (new == NULL)
			{
				return;
			}
			ci->len -= ci->len - i;
			recursive_update_to_root(t, &color_index_update_info);		
			pt->color_indices = tree_add_elt(pt->color_indices, new, &color_index_compare, &color_index_update_info);
			pt_update_color_indices(pt, f.global_char_index);
			pt_update_color_indices(pt, f.global_char_index + new->len);
			break;
		}
	}
	else
	{
		if (!pt_iterator_init(pt, &pi, f.global_char_index))
		{
			return;
		}

		for (i = 0; i < ci->len; i++)
		{
			c = pt_iterate(&pi);
			if (c != ' ')
			{
				break;
			}
		}

		if (c == '#')
		{
			ci->color = MAGENTA_TEXT;
			return;
		}
		if (chars_to_split_on[(int) c] || pt_get(pt, f.global_char_index + ci->len) == '(')
		{
			ci->color = YELLOW_TEXT;
			return;
		}
		if (pt_get_color(pt, f.global_char_index - 1) == GREEN_TEXT)
		{
			ColorIndexFinder f2;
			f2.contained = f.global_char_index - 1;
			f2.global_char_index = -1;
			ColorIndex* ci_prev = (ColorIndex*) tree_get(pt->color_indices, &f2, &color_index_finder_compare_characters);
			if (ci_prev != NULL)
			{
				PieceIterator pi2;
				if (pt_iterator_init(pt, &pi2, f.global_char_index - ci_prev->len))
				{
					bool found = false;
					for (int i = 0; i < ci_prev->len; i++)
					{
						char c = pt_iterate(&pi2);
						if (c == '\n')
						{
							found = true;
							break;
						}
					}
					if (!found)
					{
						ci->color = GREEN_TEXT;
						return;
					}
				}
			}
		}
		bool red = false;
		for (; i < ci->len; i++)
		{
			c = pt_iterate(&pi);
			if (c == ' ' || c == '\n' || c == '\0' || (c >= '*' && c <= '>')) 
			{
				red = true;
			}
			else
			{
				red = false;
				break;
			}
		}

		if (red)
		{
			ci->color = RED_TEXT;
			return;
		}

		for (; i < ci->len; i++)
		{
			c = pt_iterate(&pi);
		}
		c = pt_iterate(&pi);
		if (c == ' ')
		{
			while (c != ' ')
			{
				c = pt_iterate(&pi);
			}
			c = pt_iterate(&pi);
			if (c == '=')
			{
				c = pt_iterate(&pi);
				if (c != '=')
				{
					ci->color = BLUE_TEXT;
					return;
				}
			}
			else if (c == '*' || c== '/' || c == '+' || c == '-' || c == '^' || c == '!' || c == '|' || c == '&' || c == '!')
			{
				c = pt_iterate(&pi);
				if (c == '=')
				{
					c = pt_iterate(&pi);
					if (c != '=')
					{
						ci->color = BLUE_TEXT;
						return;
					}
				}
			}
			else if (c == '>' || c == '<')
			{
				c = pt_iterate(&pi);
				if (c == '>' || c == '<')
				{
					c = pt_iterate(&pi);
					if (c == '=')
					{
						c = pt_iterate(&pi);
						if (c != '=')
						{
							ci->color = BLUE_TEXT;
							return;
						}
					}
				}
			}
		}
		else
		{
			ci->color = CYAN_TEXT;
		}
	}
}

void color_index_update_info(Tree* t)
{
	if (t == NULL)
	{
		return;
	}
	ColorIndex* ci = (ColorIndex*) t->elt;
	if (ci == NULL)
	{
		return;
	}

	ci->chars_contained = ci->len;

	if (t->left != NULL && t->left->elt != NULL)
	{
		ci->chars_contained += ((ColorIndex*) t->left->elt)->chars_contained;
	}
	if (t->right != NULL && t->right->elt != NULL)
	{
		ci->chars_contained += ((ColorIndex*) t->right->elt)->chars_contained;
	}
}
