#include <stdlib.h>
#include <stdint.h>
#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"
#include "piece_table/undo.h"
#include "linked_list.h"
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

void print_color_index(void* v)
{
	ColorIndex* ci = (ColorIndex*) v;
	fprintf(stderr, "color: %d | len: %d | contained: %d", ci->color, ci->len, ci->chars_contained);
}

Piece* piece_create(char** text, int start_index, int len, int chars_contained)
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

void piece_free(void* v)
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
		f->global_line_index = p->lines_contained;
	}

	int left_length = 0;
	int right_length = 0;
	int right_line_length = 0;

	if (t->left != NULL && t->left->elt != NULL)
	{
		left_length = ((Piece*) t->left->elt)->chars_contained;
	}
	if (t->right != NULL && t->right->elt != NULL)
	{
		right_length = ((Piece*) t->right->elt)->chars_contained;
		right_line_length = ((Piece*) t->right->elt)->lines_contained;
	}

	if (f->contained <= left_length)
	{
		if (f->contained >= 0 && (t->left == NULL || t->left->elt == NULL))
		{
			f->global_char_index -= p->len + right_length;
			f->global_line_index -= p->lines_inside + right_line_length;
			return 0;
		}
		f->global_char_index -= p->len + right_length;
		f->global_line_index -= p->lines_inside + right_line_length;
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
		f->global_line_index -= p->lines_inside + right_line_length;
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

PieceTable* pt_create(char* buf, int len, bool do_color_indices)
{
	PieceTable* r = malloc(sizeof(PieceTable));
	if (r != NULL)
	{
		if (buf != NULL)
		{
			int new_len = len;
			for (int i = 0; i < len; i++)
			{
				if (buf[i] == '\t')
				{
					new_len += TAB_SIZE - 1;
				}
			}
			char* new_buf = malloc(sizeof(char) * new_len);
			if (new_buf == NULL)
			{
				free(r);
				return NULL;
			}
			int offset = 0;
			for (int i = 0; i < len; i++)
			{
				if (buf[i] == '\t')
				{
					for (int j = 0; j < TAB_SIZE; j++)
					{
						new_buf[i + offset + j] = ' ';
					}
					offset += TAB_SIZE - 1;
				}
				else
				{
					new_buf[i + offset] = buf[i];
				}
			}
			free(buf);
			buf = new_buf;
			len = new_len;
		}

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
			Piece* p = piece_create(&r->original, 0, len, len);
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

		r->undos = ll_create();
		if (r->undos == NULL)
		{
			pt_free(r);
			return NULL;
		}

		r->color_indices = NULL;

		if (len > 0 && do_color_indices)
		{
			ColorIndex* ci;
			ci = ci_create(CYAN_TEXT, len, len);
			if (ci == NULL)
			{
				free(r->append);
				tree_free(r->pieces, &piece_free);
				free(r);
				return NULL;
			}
			r->color_indices = tree_create(ci);
			pt_update_color_indices(r, 0);
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
			char* new_buf = malloc(sizeof(char) * (pt->append_size * 2));
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
		Piece* new_piece = piece_create(&pt->append, pt->append_len, 1, 1);
		pt->pieces = tree_create(new_piece);
		pt->append_len++;

		Undo* u = undo_rm_create(0);
		if (u != NULL)
		{
			pt_undo_update(pt, u);
		}
	}
	else
	{
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
			char* new_buf = malloc(sizeof(char) * (pt->append_size * 2));
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
			Undo* u = undo_update_create(p, index, p->start_index, p->len, p->lines_inside);
			if (u != NULL)
			{
				pt_undo_update(pt, u);
			}

			pt->append[pt->append_len] = c;
			pt->append_len++;
			p->len++;
			p->chars_contained++;
			if (c == '\n')
			{
				p->lines_contained++;
				p->lines_inside++;
			}
			tree_recursive_update_to_root(t, &piece_update_info);
		}
		else
		{
			Piece* new_one = piece_create(p->text, p->start_index, index - finder.global_char_index, index);
			if (new_one == NULL)
			{
				return;
			}
			Piece* new_two = piece_create(p->text, p->start_index + new_one->len, p->len - new_one->len, index + p->len - new_one->len);
			if (new_two == NULL)
			{
				piece_free(new_one);
				return;
			}

			p->chars_contained = finder.global_char_index + p->len;
			p->lines_contained = p->lines_inside;
			Undo* u = undo_create_create(p);
			if (u != NULL)
			{
				pt_undo_update(pt, u);
			}

			finder.contained = index;
			finder.global_char_index = -1;
			pt->pieces = tree_rm(pt->pieces, &finder, &piece_finder_compare_characters, NULL, &piece_update_info);

			if (new_one->len > 0)
			{
				Undo* u = undo_rm_create(new_one->chars_contained);
				if (u != NULL)
				{
					pt_undo_update(pt, u);
				}
				pt->pieces = tree_insert(pt->pieces, new_one, &piece_compare, &piece_update_info);
			}
			else
			{
				piece_free(new_one);
			}
			if (new_two->len > 0)
			{
				Undo* u = undo_rm_create(new_two->chars_contained);
				if (u != NULL)
				{
					pt_undo_update(pt, u);
				}
				pt->pieces = tree_insert(pt->pieces, new_two, &piece_compare, &piece_update_info);
			}
			else
			{
				piece_free(new_two);
			}

			pt->append[pt->append_len] = c;
			pt->append_len++;

			u = undo_rm_create(index + 1);
			if (u != NULL)
			{
				pt_undo_update(pt, u);
			}
			Piece* new_piece = piece_create(&pt->append, pt->append_len - 1, 1, index + 1);
			if (new_piece == NULL)
			{
				return;
			}
			pt->pieces = tree_insert(pt->pieces, new_piece, &piece_compare, &piece_update_info);
		}
	}
}

static inline void handle_piece_being_removed(PieceTable* pt, Piece* to_undo, int index)
{
	PieceFinder finder;
	finder.contained = index + 1;
	finder.global_char_index = -1;
	pt->pieces = tree_rm(pt->pieces, &finder, &piece_finder_compare_characters, NULL, &piece_update_info);

	if (to_undo != NULL)
	{
		// resetting the piece's contained so when pt_insert is called it can travel down the tree properly
		to_undo->chars_contained = finder.global_char_index + to_undo->len;
		to_undo->lines_contained = to_undo->lines_inside;
		Undo* u = undo_create_create(to_undo);
		if (u != NULL)
		{
			pt_undo_update(pt, u);
		}
	}
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
			handle_piece_being_removed(pt, p, index);
		}
		else
		{
			Undo* u = undo_update_create(p, index, p->start_index, p->len, p->lines_inside);
			if (u != NULL)
			{
				pt_undo_update(pt, u);
			}

			if ((*p->text)[p->start_index] == '\n')
			{
				p->lines_contained--;
				p->lines_inside--;
			}
			p->chars_contained--;
			p->len--;
			p->start_index++;
			tree_recursive_update_to_root(t, &piece_update_info);
		}
	}
	else if (index - finder.global_char_index == p->len - 1)
	{
		if (p->len == 1)
		{
			handle_piece_being_removed(pt, p, index);
		}
		else
		{
			Undo* u = undo_update_create(p, index - 1, p->start_index, p->len, p->lines_inside);
			if (u != NULL)
			{
				pt_undo_update(pt, u);
			}

			if ((*p->text)[p->start_index + p->len - 1] == '\n')
			{
				p->lines_contained--;
				p->lines_inside--;
			}
			p->chars_contained--;
			p->len--;
			tree_recursive_update_to_root(t, &piece_update_info);
		}
	}
	else
	{
		Piece* new_one = piece_create(p->text, p->start_index, index - finder.global_char_index, index);
		if (new_one == NULL)
		{
			return;
		}
		Piece* new_two = piece_create(p->text, p->start_index + new_one->len + 1, p->len - new_one->len - 1, index + p->len - new_one->len - 1);
		if (new_two == NULL)
		{
			piece_free(new_one);
			return;
		}

		Piece* to_remove = p;

		finder.contained = index + 1;
		finder.global_char_index = -1;
		pt->pieces = tree_rm(pt->pieces, &finder, &piece_finder_compare_characters, NULL, &piece_update_info);

		to_remove->chars_contained = finder.global_char_index + to_remove->len;
		to_remove->lines_contained = to_remove->lines_inside;
		Undo* one = undo_create_create(to_remove);
		if (one != NULL)
		{
			pt_undo_update(pt, one);
		}

		if (new_one->len > 0)
		{
			Undo* two = undo_rm_create(new_one->chars_contained);
			if (two != NULL)
			{
				pt_undo_update(pt, two);
			}
			pt->pieces = tree_insert(pt->pieces, new_one, &piece_compare, &piece_update_info);
		}
		else
		{
			piece_free(new_one);
		}
		if (new_two->len > 0)
		{
			Undo* three = undo_rm_create(new_two->chars_contained);
			if (three != NULL)
			{
				pt_undo_update(pt, three);
			}
			pt->pieces = tree_insert(pt->pieces, new_two, &piece_compare, &piece_update_info);
		}
		else
		{
			piece_free(new_two);
		}
	}
}

char pt_get(PieceTable* pt, int index)
{
	if (pt == NULL || index < 0)
	{
		return '\0';
	}

	PieceFinder finder;
	finder.contained = index + 1;
	finder.global_char_index = -1;
	Piece* p = (Piece*) tree_get(pt->pieces, &finder, &piece_finder_compare_characters);
	if (p == NULL || *p->text == NULL)
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

char pt_iterate_backwards(PieceIterator* pi)
{
	if (pi == NULL || pi->node == NULL)
	{
		return '\0';
	}

	Piece* p = (Piece*) pi->node->elt;
	char c = (*p->text)[p->start_index + pi->index];
	if (pi->index == 0)
	{
		if (pi->node->left != NULL)
		{
			pi->node = pi->node->left;
			while (pi->node->right != NULL)
			{
				pi->node = pi->node->right;
			}
		}
		else
		{
			Tree* prev = pi->node->prev;
			while (prev != NULL && prev->left == pi->node)
			{
				pi->node = prev;
				prev = prev->prev;
			}
			pi->node = prev;
		}
		if (pi->node != NULL)
		{
			pi->index = ((Piece*) pi->node->elt)->len - 1;
		}
	}
	else
	{
		pi->index--;
	}

	return c;
}

int pt_get_line_index(PieceTable* pt, int line_index)
{
	if (pt == NULL || line_index < 0)
	{
		return -1;
	}
	// if pt->pieces is null, only the 0th line is valid
	if (pt->pieces == NULL)
	{
		if (line_index > 0)
		{
			return -1;
		}
		return 0;
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

int pt_get_line_index_inverse(PieceTable* pt, int char_index)
{
	PieceFinder finder;
	finder.contained = char_index + 1;
	finder.global_char_index = -1;
	Piece* p = (Piece*) tree_get(pt->pieces, &finder, &piece_finder_compare_characters);
	if (p == NULL)
	{
		return -1;
	}

	int r = finder.global_line_index;
	for (int i = p->start_index; i < p->start_index + p->len && i < p->start_index + (char_index - finder.global_char_index); i++)
	{
		if ((*p->text)[i] == '\n')
		{
			r++;
		}
	}

	return r;
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
	tree_free(pt->pieces, &piece_free);
	tree_free(pt->color_indices, &free);

	while (1)
	{
		LinkedList* undos = ll_rm(pt->undos, 0);

		if (undos == NULL)
		{
			break;
		}

		while (1)
		{
			Undo* to_free = (Undo*) ll_rm(undos, 0);
			if (to_free == NULL)
			{
				break;
			}
			undo_free(to_free);
		}
		ll_free(undos);
	}
	ll_free(pt->undos);

	free(pt);
}

char* pt_flatten_to_str(PieceTable* pt)
{
	char* buf = malloc(sizeof(char) * ((Piece*) pt->pieces->elt)->chars_contained + 1);
	if (buf != NULL)
	{
		PieceIterator pi;
		if (pt_iterator_init(pt, &pi, 0))
		{
			char c = pt_iterate(&pi);
			int i = 0;
			for (; c != '\0'; i++, c = pt_iterate(&pi))
			{
				buf[i] = c;
			}
			buf[i] = '\0';
			return buf;
		}
		else
		{
			free(buf);
			return NULL;
		}
	}
	return buf;
}

void piece_iterator_copy(PieceIterator* to, PieceIterator* from)
{
	if (to == NULL || from == NULL)
	{
		return;
	}

	to->node = from->node;
	to->index = from->index;
}
