#include <stdlib.h>
#include <stdint.h>
#include "piece_table.h"
#include "piece_table_color_indices.h"
#include "tree.h"
#include "global.h"

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

void ci_init_arrays(void)
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

static void update_until_no_update_occurs(PieceTable* pt, ColorIndexFinder f, int len)
{
	// updating every color index moving to the right until we call pt_update_color_indices and the color doesn't change, meaning we are done
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

void ci_handle_insert(PieceTable* pt, int index)
{
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

	update_until_no_update_occurs(pt, f, len);
}

void ci_handle_rm(PieceTable* pt, int index)
{
	ColorIndexFinder f;
	f.contained = index + 1;
	f.global_char_index = -1;
	Tree* t = tree_helper(pt->color_indices, &f, &ci_finder_compare_characters);
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

	update_until_no_update_occurs(pt, f, len);
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

// if index points to an existing color index that is properly formatted, then the color of that color index will be updated
// if index points to a color index that needs to be broken up (ex: if the color index contains the text "int x", this needs to be split into "int" and "x")
// then the index will be split into two, which will both be updated using recursion
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
