#include <stdlib.h>
#include <stdint.h>
#include "piece_table.h"
#include "global.h"
#include "tree.h"

// TODO: use bitflags intead of bool array
static int prime_numbers[NUM_PRIME_NUMBERS];
static bool chars_to_split_on[NUM_CHARS];
static bool control_word_check[MAX_HASH_VALUE];

static int hash_function(const char* s, int len)
{
	uint64_t val = 0;
	for (int i = 0; i < len; i++)
	{
		val += ((int) s[i]) * prime_numbers[i % NUM_PRIME_NUMBERS];
	}
	return (int) (val % MAX_HASH_VALUE);
}

void pt_init_arrays(void)
{
	for (int i = 0; i < NUM_CHARS; i++)
	{
		chars_to_split_on[i] = false;
	}
	chars_to_split_on[' '] = true;
	chars_to_split_on['\n'] = true;
	chars_to_split_on['['] = true;
	chars_to_split_on[']'] = true;
	chars_to_split_on['{'] = true;
	chars_to_split_on['}'] = true;
	chars_to_split_on['('] = true;
	chars_to_split_on[')'] = true;
	chars_to_split_on[';'] = true;
	chars_to_split_on[','] = true;
	prime_numbers[0] = 67;
	prime_numbers[1] = 283;
	prime_numbers[2] = 31;
	prime_numbers[3] = 593;
	prime_numbers[4] = 379;
	prime_numbers[5] = 389;
	prime_numbers[6] = 821;
	prime_numbers[7] = 113;
	for (int i = 0; i < MAX_HASH_VALUE; i++)
	{
		control_word_check[i] = false;
	}
	control_word_check[hash_function("break", 5)] = true;
	control_word_check[hash_function("case", 4)] = true;
	control_word_check[hash_function("continue", 8)] = true;
	control_word_check[hash_function("default", 7)] = true;
	control_word_check[hash_function("do", 2)] = true;
	control_word_check[hash_function("else", 4)] = true;
	control_word_check[hash_function("for", 3)] = true;
	control_word_check[hash_function("goto", 4)] = true;
	control_word_check[hash_function("if", 2)] = true;
	control_word_check[hash_function("return", 6)] = true;
	control_word_check[hash_function("switch", 6)] = true;
	control_word_check[hash_function("typedef", 7)] = true;
	control_word_check[hash_function("while", 5)] = true;
}

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

PieceTable* pt_create(char* buf, int len)
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
		if (len > 0)
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
		else
		{
			r->color_indices = NULL;
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
		Piece* new_piece = piece_create(&pt->append, pt->append_len, 1, 1);
		pt->pieces = tree_create(new_piece);
		pt->append_len++;
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

			finder.contained = index;
			pt->pieces = tree_rm(pt->pieces, &finder, &piece_finder_compare_characters, &piece_free, &piece_update_info);

			if (new_one->len > 0)
			{
				pt->pieces = tree_add_elt(pt->pieces, new_one, &piece_compare, &piece_update_info);
			}
			else
			{
				piece_free(new_one);
			}
			if (new_two->len > 0)
			{
				pt->pieces = tree_add_elt(pt->pieces, new_two, &piece_compare, &piece_update_info);
			}
			else
			{
				piece_free(new_two);
			}

			pt->append[pt->append_len] = c;
			pt->append_len++;

			Piece* new_piece = piece_create(&pt->append, pt->append_len - 1, 1, index + 1);
			if (new_piece == NULL)
			{
				return;
			}
			pt->pieces = tree_add_elt(pt->pieces, new_piece, &piece_compare, &piece_update_info);
		}
	}

	// we need to add the character we just added to a color index
	// if we pick the ci to the right of the character and its a char to split on,
	// it will get split off and be a single character, which will fragment the ci's
	// and lead to them not getting highlighted properly
	// also, if we just inserted a character at the end of the document, then we need to get the colorindex
	// one to the left of the character we just inserted, because if we try to get the ci at the character we just inserted,
	// tree_get will return NULL (there is not ci at the index of the character we just inserted)
	ColorIndexFinder f;
	if (chars_to_split_on[(int) pt_get(pt, index + 1)] || pt_get(pt, index + 1) == '\0')
	{
		f.contained = index;
	}
	else
	{
		f.contained = index + 1;
	}

	f.global_char_index = -1;
	Tree* t = tree_helper(pt->color_indices, &f, &ci_finder_compare_characters);
	if (t == NULL)
	{
		if (pt->pieces != NULL && pt->pieces->elt != NULL)
		{
			// this has to be one less than pt->pieces because we are going to add one later
			ColorIndex* ci = ci_create(CYAN_TEXT, ((Piece*) pt->pieces->elt)->chars_contained - 1, ((Piece*) pt->pieces->elt)->chars_contained - 1);
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

	int len = ((ColorIndex*) t->elt)->len;

	tree_recursive_update_to_root(t, &ci_update_info);
	pt_update_color_indices(pt, index);
	if (f.global_char_index > 1)
	{
		pt_update_color_indices(pt, f.global_char_index - 1);
	}
	while (true)
	{
		f.contained = f.global_char_index + len + 1;
		f.global_char_index = -1;
		ColorIndex* ci = (ColorIndex*) tree_get(pt->color_indices, &f, &ci_finder_compare_characters);
		if (ci == NULL)
		{
			break;
		}
		int c1 = ci->color;
		pt_update_color_indices(pt, f.global_char_index);
		f.contained = f.global_char_index + 1;
		f.global_char_index = -1;
		ci = (ColorIndex*) tree_get(pt->color_indices, &f, &ci_finder_compare_characters);
		if (ci == NULL)
		{
			break;
		}
		int c2 = ci->color;
		if (c1 == c2)
		{
			break;
		}
		len = ci->len;
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
			finder.contained = index + 1;
			pt->pieces = tree_rm(pt->pieces, &finder, &piece_finder_compare_characters, &piece_free, &piece_update_info);
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
			tree_recursive_update_to_root(t, &piece_update_info);
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
			pt->pieces = tree_rm(pt->pieces, &finder, &piece_finder_compare_characters, &piece_free, &piece_update_info);
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

		finder.contained = index + 1;
		pt->pieces = tree_rm(pt->pieces, &finder, &piece_finder_compare_characters, &piece_free, &piece_update_info);

		if (new_one->len > 0)
		{
			pt->pieces = tree_add_elt(pt->pieces, new_one, &piece_compare, &piece_update_info);
		}
		else
		{
			piece_free(new_one);
		}
		if (new_two->len > 0)
		{
			pt->pieces = tree_add_elt(pt->pieces, new_two, &piece_compare, &piece_update_info);
		}
		else
		{
			piece_free(new_two);
		}
	}

	ColorIndexFinder f;
	f.contained = index + 1;
	f.global_char_index = -1;
	t = tree_helper(pt->color_indices, &f, &ci_finder_compare_characters);
	if (t == NULL)
	{
		if (pt->pieces != NULL && pt->pieces->elt != NULL)
		{
			// this has to be one more than pt->pieces because we are going to subtract one later
			ColorIndex* ci = ci_create(CYAN_TEXT, ((Piece*) pt->pieces->elt)->chars_contained + 1, ((Piece*) pt->pieces->elt)->chars_contained + 1);
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

	int len = ((ColorIndex*) t->elt)->len;
	if (len == 1)
	{
		f.contained = index + 1;
		f.global_char_index = -1;
		pt->color_indices = tree_rm(pt->color_indices, &f, &ci_finder_compare_characters, &free, &ci_update_info);
		len = 0;
	}
	else
	{
		int index_to_update = index;
		if (f.global_char_index + len - 1 == index_to_update)
		{
			index_to_update--;
		}
		((ColorIndex*) t->elt)->len--;
		((ColorIndex*) t->elt)->chars_contained--;
		len--;
		tree_recursive_update_to_root(t, &ci_update_info);
		pt_update_color_indices(pt, index_to_update);
	}

	if (f.global_char_index > 1)
	{
		pt_update_color_indices(pt, f.global_char_index - 1);
	}
	while (true)
	{
		f.contained = f.global_char_index + len + 1;
		f.global_char_index = -1;
		ColorIndex* ci = (ColorIndex*) tree_get(pt->color_indices, &f, &ci_finder_compare_characters);
		if (ci == NULL)
		{
			break;
		}
		int c1 = ci->color;
		pt_update_color_indices(pt, f.global_char_index);
		f.contained = f.global_char_index + 1;
		f.global_char_index = -1;
		ci = (ColorIndex*) tree_get(pt->color_indices, &f, &ci_finder_compare_characters);
		if (ci == NULL)
		{
			break;
		}
		int c2 = ci->color;
		if (c1 == c2)
		{
			break;
		}
		len = ci->len;
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

int pt_get_line_index(PieceTable* pt, int line_index)
{
	if (pt == NULL || line_index < 0)
	{
		return -1;
	}
	if (pt->pieces == NULL)
	{
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
	free(pt);
}

int ci_compare(Tree* t, void* elt)
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

int ci_finder_compare_characters(Tree* t, void* elt)
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
	if (pt == NULL || pt->color_indices == NULL || index < 0)
	{
		return WHITE_TEXT;
	}

	ColorIndexFinder f;
	f.contained = index + 1;
	f.global_char_index = -1;
	ColorIndex* ci = (ColorIndex*) tree_get(pt->color_indices, &f, &ci_finder_compare_characters);
	if (ci == NULL)
	{
		return WHITE_TEXT;
	}
	return ci->color;
}

ColorIndex* ci_create(int color, int len, int chars_contained)
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

	Tree* t = tree_helper(pt->color_indices, &f, &ci_finder_compare_characters);
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

	bool split = false;

	int i;
	char c;
	if (ci->len > 1)
	{
		for (i = 0; i < ci->len; i++)
		{
			c = pt_iterate(&pi);
			if (chars_to_split_on[(int) c])
			{
				if ((c == ' ' || c == '\n') && (i == 0 || i == ci->len - 1))
				{
					continue;
				}
				split = true;
				break;
			}
		}
	}

	if (split)
	{
		if (c == ' ' || c == '\n')
		{
			ColorIndex* new = ci_create(ci->color, ci->len - i, f.global_char_index + ci->len);
			if (new == NULL)
			{
				return;
			}
			ci->len -= ci->len - i;
			tree_recursive_update_to_root(t, &ci_update_info);		
			pt->color_indices = tree_add_elt(pt->color_indices, new, &ci_compare, &ci_update_info);
			pt_update_color_indices(pt, f.global_char_index);
			pt_update_color_indices(pt, f.global_char_index + ci->len);
		}
		else
		{
			ColorIndex* new_one = ci_create(ci->color, 1, f.global_char_index + i + 1);
			if (new_one == NULL)
			{
				return;
			}
			ColorIndex* new_two = ci_create(ci->color, ci->len - i - 1, f.global_char_index + ci->len);
			if (new_two == NULL)
			{
				free(new_one);
				return;
			}

			ci->len -= ci->len - i;
			tree_recursive_update_to_root(t, &ci_update_info);

			pt->color_indices = tree_add_elt(pt->color_indices, new_one, &ci_compare, &ci_update_info);
			if (new_two->len > 0)
			{
				pt->color_indices = tree_add_elt(pt->color_indices, new_two, &ci_compare, &ci_update_info);
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
			if (c != ' ' && c != '\n')
			{
				break;
			}
		}

		if (pt_get_color(pt, f.global_char_index - 1) == GREEN_TEXT)
		{
			ColorIndexFinder f2;
			f2.contained = f.global_char_index;
			f2.global_char_index = -1;
			ColorIndex* ci_prev = (ColorIndex*) tree_get(pt->color_indices, &f2, &ci_finder_compare_characters);
			if (ci_prev != NULL)
			{
				PieceIterator pi2;
				if (pt_iterator_init(pt, &pi2, f.global_char_index - ci_prev->len))
				{
					char c = pt_iterate(&pi2);
					while (c == '\n')
					{
						c = pt_iterate(&pi2);
					}

					bool found = false;
					for (int i = 0; i < ci_prev->len; i++)
					{
						c = pt_iterate(&pi2);
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
		else if (c == '/' && (pt_get(pt, f.global_char_index + i + 1) == '/' || pt_get(pt, f.global_char_index + i - 1) == '/'))
		{
			ci->color = GREEN_TEXT;
			return;
		}

		if (!pt_iterator_init(pt, &pi, f.global_char_index))
		{
			return;
		}

		for (i = 0; i < ci->len; i++)
		{
			c = pt_iterate(&pi);
			if (c != ' ' && c != '\n')
			{
				break;
			}
		}

		if (c == '#')
		{
			ci->color = MAGENTA_TEXT;
			return;
		}
		int len = ci->len - i;
		if (len > 0)
		{
			char buf[len];
			for (i = 0; i < len && c != ' ' && c != '\n' && c != '\0'; i++)
			{
				buf[i] = c;
				c = pt_iterate(&pi);
			}
			if (control_word_check[hash_function(&buf[0], i)])
			{
				ci->color = MAGENTA_TEXT;
				return;
			}
		}

		if (!pt_iterator_init(pt, &pi, f.global_char_index))
		{
			return;
		}

		for (i = 0; i < ci->len; i++)
		{
			c = pt_iterate(&pi);
			if (c != ' ' && c != '\n')
			{
				break;
			}
		}

		if (chars_to_split_on[(int) c] || pt_get(pt, f.global_char_index + ci->len) == '(')
		{
			ci->color = YELLOW_TEXT;
			return;
		}

		bool red = false;
		for (; i < ci->len; i++)
		{
			if (c == ' ' || c == '\n' || c == '\0' || (c >= '*' && c <= '>')) 
			{
				red = true;
			}
			else
			{
				red = false;
				break;
			}
			c = pt_iterate(&pi);
		}

		if (red)
		{
			ci->color = RED_TEXT;
			return;
		}

		if (!pt_iterator_init(pt, &pi, f.global_char_index))
		{
			return;
		}

		for (i = 0; i < ci->len; i++)
		{
			c = pt_iterate(&pi);
			if (c != ' ' && c != '\n')
			{
				break;
			}
		}

		for (; i < ci->len; i++)
		{
			c = pt_iterate(&pi);
		}
		if (c == ' ')
		{
			c = pt_iterate(&pi);
			while (c == '*')
			{
				c = pt_iterate(&pi);
			}
			if (c == '(' || c == ')' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
			{
				ci->color = BLUE_TEXT;
				return;
			}
		}
		else
		{
			ci->color = CYAN_TEXT;
		}
	}
}

void ci_update_info(Tree* t)
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
