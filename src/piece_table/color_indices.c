#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"
#include "tree.h"
#include "global.h"

#define MAX_HASH_VALUE 2000
#define NUM_CONTROL_WORDS 13

static bool operator_chars[NUM_CHARS];
static bool yellow_chars[NUM_CHARS];
static bool control_chars[NUM_CHARS];
static LinkedList* control_word_check[MAX_HASH_VALUE];
static char* control_words[] = {"break", "case", "continue", "default", "do", "else", "for", "goto", "if", "return", "switch", "typedef", "while"};

bool is_control_char(char c)
{
	return control_chars[(int) c];
}

bool is_control_word(char* s)
{
	LinkedList* lst = control_word_check[hash_function(s, MAX_HASH_VALUE)];
	if (lst != NULL)
	{
		for (int i = 0; i < lst->size; i++)
		{
			char* str = ll_get_elt(lst, i);
			if (str != NULL)
			{
				if (!strcmp(s, str))
				{
					return true;
				}
			}
		}
	}
	return false;
}

void ci_init_arrays(void)
{
	for (int i = 0; i < NUM_CHARS; i++)
	{
		operator_chars[i] = false;
		yellow_chars[i] = false;
		control_chars[i] = false;
	}
	yellow_chars['['] = true;
	yellow_chars[']'] = true;
	yellow_chars['{'] = true;
	yellow_chars['}'] = true;
	yellow_chars['('] = true;
	yellow_chars[')'] = true;
	yellow_chars[','] = true;
	for (int i = '*'; i <= '>'; i++)
	{
		operator_chars[i] = true;
	}
	for (int i = '0'; i <= '9'; i++)
	{
		operator_chars[i] = false;
	}
	operator_chars['!'] = true;
	operator_chars['&'] = true;
	operator_chars['%'] = true;
	operator_chars['^'] = true;
	operator_chars['|'] = true;
	operator_chars['~'] = true;
	control_chars[';'] = true;
	control_chars['{'] = true;
	control_chars['}'] = true;
	for (int i = 0; i < MAX_HASH_VALUE; i++)
	{
		control_word_check[i] = NULL;
	}
	for (int i = 0; i < NUM_CONTROL_WORDS; i++)
	{
		if (control_word_check[hash_function(control_words[i], MAX_HASH_VALUE)] == NULL)
		{
			control_word_check[hash_function(control_words[i], MAX_HASH_VALUE)] = ll_create();
		}
		ll_insert(control_word_check[hash_function(control_words[i], MAX_HASH_VALUE)], control_words[i], 0);
	}
}
void ci_uninit_arrays(void)
{
	for (int i = 0; i < NUM_CONTROL_WORDS; i++)
	{
		LinkedList* lst = control_word_check[hash_function(control_words[i], MAX_HASH_VALUE)];
		if (lst != NULL)
		{
			while (lst->size > 0)
			{
				ll_rm(lst, 0);
			}
			ll_free(lst);
		}
	}
}

void update_until_no_update_occurs(PieceTable* pt, ColorIndexFinder f, int len)
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

void ci_prepare(PieceTable* pt, int index)
{
	if (pt != NULL)
	{
		pt->ci_index = index;
		pt->ci_chars_added = 0;
	}
}

void ci_handle_insert(PieceTable* pt)
{
	if (pt == NULL)
	{
		return;
	}

	ColorIndexFinder f;
	// not ci_index + 1 because the inserted character does not exist in the ci's yet, thus if there is a ci
	// for recently inserted characters it will be to the left of the index
	f.contained = pt->ci_index + pt->ci_chars_added;
	f.global_char_index = -1;

	Tree* t;
	if (pt->color_indices == NULL)
	{
		ColorIndex* ci = ci_create(WHITE_TEXT, 0, 0);
		if (ci == NULL)
		{
			return;
		}
		t = tree_create(ci);
		if (t == NULL)
		{
			free(ci);
		}
		pt->color_indices = t;
	}
	else
	{
		t = tree_helper(pt->color_indices, &f, &ci_finder_compare_characters);
	}
	if (t == NULL)
	{
		return;
	}
	ColorIndex* ci = t->elt;
	if (ci == NULL)
	{
		return;
	}

	if (ci->color == WHITE_TEXT)
	{
		ci->len++;
		tree_recursive_update_to_root(t, &ci_update_info);
	}
	else
	{
		ColorIndex* new = ci_create(WHITE_TEXT, 1, pt->ci_index + 1);
		if (new != NULL)
		{
			if (pt->ci_index == f.global_char_index + ci->len)
			{
				pt->color_indices = tree_insert(pt->color_indices, new, &ci_compare, &ci_update_info);
			}
			else
			{
				ColorIndex* new2 = ci_create(ci->color, pt->ci_index - f.global_char_index, pt->ci_index);
				if (new2 != NULL)
				{
					if (ci->len - new2->len > 0)
					{
						ci->len -= new2->len;
						tree_recursive_update_to_root(t, &ci_update_info);
					}
					else
					{
						f.contained = f.global_char_index + 1;
						f.global_char_index = -1;
						pt->color_indices = tree_rm(pt->color_indices, &f, &ci_compare, &free, &ci_update_info);
					}

					pt->color_indices = tree_insert(pt->color_indices, new2, &ci_compare, &ci_update_info);
					pt->color_indices = tree_insert(pt->color_indices, new, &ci_compare, &ci_update_info);
				}
				else
				{
					free(new);
					return;
				}
			}
		}
		else
		{
			return;
		}
	}

	pt->ci_chars_added++;
}

void ci_handle_rm(PieceTable* pt)
{
	if (pt == NULL)
	{
		return;
	}

	ColorIndexFinder f;
	f.contained = pt->ci_index + pt->ci_chars_added;
	f.global_char_index = -1;

	Tree* t = tree_helper(pt->color_indices, &f, &ci_finder_compare_characters);
	if (t == NULL)
	{
		return;
	}
	ColorIndex* ci = t->elt;
	if (ci == NULL)
	{
		return;
	}

	if (ci->len == 1)
	{
		f.contained = pt->ci_index + pt->ci_chars_added;
		f.global_char_index = -1;
		pt->color_indices = tree_rm(pt->color_indices, &f, &ci_finder_compare_characters, &free, &ci_update_info);
	}
	else
	{
		ci->len--;
		tree_recursive_update_to_root(t, &ci_update_info);
	}

	if (pt->ci_chars_added == 0)
	{
		pt->ci_index--;
	}
	else
	{
		pt->ci_chars_added--;
	}
}

void ci_handle_multiple_rm(PieceTable* pt, int num_chars_removing);
{
	if (pt == NULL || pt->pieces == NULL || pt->pieces->elt == NULL || num_chars_removing <= 0)
	{
		return;
	}

	ColorIndexFinder f;
	f.contained = pt->ci_index + 1;
	f.global_char_index = -1;

	Tree* t = tree_helper(pt->pieces, &f, &ci_finder_compare_characters);
	if (t == NULL)
	{
		return;
	}

	ColorIndex* ci = t->elt;
	if (ci == NULL)
	{
		return;
	}

	int num_chars_removed_this_iteration;

	if (ci->len > num_chars_removing)
	{
		ci->len -= num_chars_removing;
		num_chars_removed_this_iteration = num_chars_removing;
		tree_recursive_update_to_root(t, &ci_update_info);
	}
	else (ci->len <= num_chars_removing)
	{
		num_chars_removed_this_iteration = ci->len;

		f.contained = pt->ci_index + 1;
		f.global_char_index = -1;
		pt->color_indices = tree_rm(pt->color_indices, &f, &ci_finder_compare_characters, &free, &ci_update_info);
	}

	num_chars_removing -= num_chars_removed_this_iteration;

	if (pt->ci_chars_added - num_chars_removed_this_iteration < 0)
	{
		num_chars_removed_this_iteration -= pt->ci_chars_added;
		pt->ci_chars_added = 0;
		pt->ci_index -= num_chars_removed_this_iteration;
	}
	else
	{
		pt->ci_chars_added -= num_chars_removed_this_iteration;
	}


	ci_handle_rm_on_boundary(pt, num_chars_removing);
}

static char inside_comment_or_quote(PieceTable* pt, int index)
{
	PieceIterator pi;
	if (!pt_iterator_init(pt, &pi, index))
	{
		return '\0';
	}

	bool inside_double_quote = false;
	bool inside_single_quote = false;
	bool inside_comment = false;

	char c = pt_iterate_backwards(&pi);
	while (c != '\n' && c != '\0')
	{
		switch (c)
		{
			case '"':
			{
				inside_double_quote = !inside_double_quote;
				break;
			}
			case '\'':
			{
				inside_single_quote = !inside_single_quote;
				break;
			}
			case '/':
			{
				c = pt_iterate_backwards(&pi);
				if (c == '/')
				{
					inside_comment = true;
				}
				break;
			}
		}

		if (inside_comment)
		{
			break;
		}

		c = pt_iterate_backwards(&pi);
	}

	if (inside_comment)
	{
		return '/';
	}
	if (inside_double_quote)
	{
		return '"';
	}
	if (inside_single_quote)
	{
		return '\'';
	}
	return '\0';
}

static int iterate_to_bound_of_update_chunk(PieceTable* pt, int index, char (*iterate)(PieceIterator*), int delta)
{
	if (pt == NULL || pt->pieces == NULL || iterate == NULL || (delta != -1 && delta != 1))
	{
		return -1;
	}
	int pt_len = ((Piece*) pt->pieces->elt)->chars_contained;

	if (index < 0)
	{
		index = 0;
	}
	if (index >= pt_len)
	{
		index = pt_len - 1;
	}

	PieceIterator pi;
	if (!pt_iterator_init(pt, &pi, index))
	{
		return -1;
	}

	char c = (*iterate)(&pi);
	if (control_chars[(int) c] && index > 0 && delta == -1)
	{
		c = (*iterate)(&pi);
		index += delta;
	}
	while (index >= 0 && index < pt_len)
	{
		if (control_chars[(int) c])
		{
			char result = inside_comment_or_quote(pt, index);
			switch (result)
			{
				case '\'':
				{
					while (index >= 0 && index < pt_len && c != '\'')
					{
						c = (*iterate)(&pi);
						index += delta;
					}
					break;
				}

				case '"':
				{
					while (index >= 0 && index < pt_len && c != '"')
					{
						c = (*iterate)(&pi);
						index += delta;
					}
					break;
				}

				case '/':
				{
					while (index >= 0 && index < pt_len)
					{
						if (c == '/')
						{
							c = (*iterate)(&pi);
							index += delta;
							if (c == '/')
							{
								break;
							}
						}
						else if (c == '\n')
						{
							break;
						}
						c = (*iterate)(&pi);
						index += delta;
					}
					break;
				}
			}

			break;
		}
		index += delta;
		c = (*iterate)(&pi);
	}

	if (delta == -1)
	{
		index++;
	}
	else
	{
		if (c == '\0')
		{
			index--;
		}
	}

	return index;
}

int iterate_to_start_of_update_chunk(PieceTable* pt, int start_index)
{
	return iterate_to_bound_of_update_chunk(pt, start_index, &pt_iterate_backwards, -1);
}

int iterate_to_end_of_update_chunk(PieceTable* pt, int end_index)
{
	return iterate_to_bound_of_update_chunk(pt, end_index, &pt_iterate, 1);
}

bool ci_execute(PieceTable* pt, int* first_line_updated, int* last_line_updated)
{
	if (pt == NULL)
	{
		return false;
	}

	int start_index = iterate_to_start_of_update_chunk(pt, pt->ci_index);
	if (start_index < 0)
	{
		return false;
	}

	int end_index = iterate_to_end_of_update_chunk(pt, pt->ci_index + pt->ci_chars_added - 1);
	if (end_index < 0)
	{
		return false;
	}

	merge_color_indices_on_boundary(pt, start_index, end_index);
	pt_update_color_indices(pt, start_index);
	if (first_line_updated != NULL)
	{
		*first_line_updated = pt_get_line_index_inverse(pt, start_index);
	}
	if (last_line_updated != NULL)
	{
		*last_line_updated = pt_get_line_index_inverse(pt, end_index);
	}

	return true;
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

// expects already_present to be in the tree, and f to be a ColorIndexFinder which was used to retrieve already_present from the color indices tree in pt
// return true if already_present remains in the tree, false if it was removed
static bool ci_add_and_subtract(PieceTable* pt, Tree* already_present_tree, ColorIndex* already_present, ColorIndex* to_add, ColorIndexFinder* f)
{
	if (already_present->len - to_add->len > 0)
	{
		already_present->len -= to_add->len;
		tree_recursive_update_to_root(already_present_tree, &ci_update_info);
		pt->color_indices = tree_insert(pt->color_indices, to_add, &ci_compare, &ci_update_info);
		f->global_char_index += to_add->len;
		return true;
	}
	else
	{
		f->contained = f->global_char_index + 1;
		pt->color_indices = tree_rm(pt->color_indices, f, &ci_finder_compare_characters, &free, &ci_update_info);
		pt->color_indices = tree_insert(pt->color_indices, to_add, &ci_compare, &ci_update_info);
		return false;
	}
}

static bool only_contains_parenthesis_or_alphanumeric(PieceTable* pt, int start_index, int len)
{
	PieceIterator pi;
	if (pt_iterator_init(pt, &pi, start_index))
	{
		for (int i = 0; i < len; i++)
		{
			char c = pt_iterate(&pi);
			if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')))
			{
				return false;
			}
			else if (c == '\0')
			{
				return true;
			}
		}
	}

	return true;
}

#define FUNCTION_SIGNATURE 1
#define ARITHMETIC 2

// updates a color index that is guranteed to not need to handle comments, quotes, or macros
static void pt_update_color_indices_helper(PieceTable* pt, int index)
{
	if (pt == NULL || index < 0 || pt->color_indices == NULL)
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
	ColorIndex* ci = t->elt;
	if (ci == NULL)
	{
		return;
	}

	PieceIterator pi;
	if (!pt_iterator_init(pt, &pi, f.global_char_index))
	{
		return;
	}

	bool function_declaration = false;
	int i = 0;
	char c = pt_iterate(&pi);
	while (i < ci->len)
	{
		while ((c == ' ' || c == '\n') && i < ci->len)
		{
			c = pt_iterate(&pi);
			i++;
		}

		if (c >= '0' && c <= '9')
		{
			while (c >= '0' && c <= '9' && i < ci->len)
			{
				c = pt_iterate(&pi);
				i++;
			}

			ColorIndex* new = ci_create(RED_TEXT, i, f.global_char_index + i);
			if (new != NULL)
			{
				if (!ci_add_and_subtract(pt, t, ci, new, &f))
				{
					break;
				}
				i = 0;
			}
			else
			{
				break;
			}
		}
		if (yellow_chars[(int) c])
		{
			while (yellow_chars[(int) c] && i < ci->len)
			{
				if (control_chars[(int) c])
				{
					function_declaration = false;
				}
				c = pt_iterate(&pi);
				i++;
			}


			ColorIndex* new = ci_create(YELLOW_TEXT, i, f.global_char_index + i);
			if (new != NULL)
			{
				if (!ci_add_and_subtract(pt, t, ci, new, &f))
				{
					break;
				}
				i = 0;
			}
			else
			{
				break;
			}

			// because (x) *y could be x * y if both x and y are the same type, or a typecast of a derefernce of y if 
			// x is a type and y is a pointer, i am hardcoding any single non astrix prefixed valid name surrounded in parenthesis to be blue.
			// this is the only solution i can think of besides tracking the types of variables, which will be significantly more code and overhead
			if (pt_get(pt, f.global_char_index - 1) == '(')
			{
				PieceIterator pi2;
				ColorIndexFinder f2;
				f2.contained = f.global_char_index - 1;
				f2.global_char_index = -1;
				ColorIndex* prev = tree_get(pt->color_indices, &f2, &ci_finder_compare_characters);
				if (prev != NULL)
				{
					int j = 0;
					char c2 = '\0';
					if (f.global_char_index - 2 > 0 && pt_iterator_init(pt, &pi2, f.global_char_index - 2))
					{
						j = f.global_char_index - 2 - f2.global_char_index;
						c2 = pt_iterate_backwards(&pi2);
						while ((c2 == ' ' || c2 == '\n') && j >= 0)
						{
							c2 = pt_iterate_backwards(&pi2);
							j--;
						}
					}
					if (!is_valid_name_character(c2) && pt_iterator_init(pt, &pi2, f.global_char_index))
					{
						j = 0;
						char c2 = pt_iterate(&pi2);
						while ((is_valid_name_character(c2) || c2 == ' ' || c2 == '\n') && c2 != '*' && c2 != '[' && c2 != ']' && j < ci->len)
						{
							c2 = pt_iterate(&pi2);
							j++;
						}
						while ((c2 == '*' || c2 == '[' || c2 == ']') && j < ci->len)
						{
							c2 = pt_iterate(&pi2);
							j++;
						}
						if (c2 == ')')
						{
							ColorIndex* new2 = ci_create(BLUE_TEXT, j, f.global_char_index + j);
							if (!ci_add_and_subtract(pt, t, ci, new2, &f))
							{
								break;
							}
							c = c2;
							piece_iterator_copy(&pi, &pi2);
							// no need to reset i to 0 because it never changed (we used j)
						}
					}
				}
			}
		}
		if (is_valid_name_character(c) && c != '*' && c != '[' && c != ']')
		{
			char buf[CONTROL_WORD_MAX_LENGTH];
			int buf_index = 0;
			while (is_valid_name_character(c) && c != '[' && c != ']' && i < ci->len)
			{
				if (buf_index < CONTROL_WORD_MAX_LENGTH - 1)
				{
					buf[buf_index] = c;
					buf_index++;
				}
				else
				{
					// convers the case where a function name is a control word for the first <CONTROL_WORD_MAX_LENGTH - 1> characters
					buf[0] = '\0';
				}
				c = pt_iterate(&pi);
				i++;
			}
			buf[buf_index] = '\0';

			int color;

			if (is_control_word(buf))
			{
				color = MAGENTA_TEXT;
			}
			else if (function_declaration)
			{
				ColorIndexFinder f2;
				f2.contained = f.global_char_index;
				f2.global_char_index = -1;
				ColorIndex* prev = tree_get(pt->color_indices, &f2, &ci_finder_compare_characters);
				if (prev != NULL && (prev->color == CYAN_TEXT || prev->color == BLUE_TEXT))
				{
					prev->color = BLUE_TEXT;
					color = CYAN_TEXT;
				}
				else
				{
					color = BLUE_TEXT;

					if (prev != NULL)
					{
						PieceIterator pi2;
						if (pt_iterator_init(pt, &pi2, f2.global_char_index + prev->len - 1))
						{
							char c2 = pt_iterate_backwards(&pi2);
							while (c2 == ' ' || c2 == '\n')
							{
								c2 = pt_iterate_backwards(&pi2);
							}
							if (c2 == '*')
							{
								color = CYAN_TEXT;
							}
						}
					}
				}
			}
			else
			{
				color = CYAN_TEXT;

				while ((c == ' ' || c == '\n') && i < ci->len)
				{
					c = pt_iterate(&pi);
					i++;
				}

				ColorIndexFinder f2;
				f2.contained = f.global_char_index;
				f2.global_char_index = -1;
				ColorIndex* prev = tree_get(pt->color_indices, &f2, &ci_finder_compare_characters);
				ColorIndex* prev_prev = NULL;
				while (prev_prev != prev && prev != NULL && prev->color == YELLOW_TEXT && only_contains_parenthesis_or_alphanumeric(pt, f2.global_char_index, prev->len))
				{
					f2.contained = f2.global_char_index;
					f2.global_char_index = -1;
					prev_prev = prev;
					prev = tree_get(pt->color_indices, &f2, &ci_finder_compare_characters);
				}

				if (prev != NULL && prev->color == CYAN_TEXT)
				{
					PieceIterator pi2;
					if (pt_iterator_init(pt, &pi2, f2.global_char_index))
					{
						char c2 = pt_iterate(&pi2);
						int j = 0;
						while ((c2 == ' ' || c2 == '\n') && j < prev->len)
						{
							c2 = pt_iterate(&pi2);
							j++;
						}
						if (pt_get(pt, f2.global_char_index + j - 1) != '*')
						{
							prev->color = BLUE_TEXT;
						}
					}
				}

				if (c == '(')
				{
					color = YELLOW_TEXT;

					PieceIterator pi2;
					int j = i + 1;
					if (pt_iterator_init(pt, &pi2, f.global_char_index + j))
					{
						char c2 = pt_iterate(&pi2);
						while (c2 != ')' && !control_chars[(int) c] && j < ci->len)
						{
							c2 = pt_iterate(&pi2);
							j++;
						}
						if (c2 == ')')
						{
							c2 = pt_iterate(&pi2);
							j++;
							while ((c2 == ' ' || c2 == '\n') && j < ci->len)
							{
								c2 = pt_iterate(&pi2);
								j++;
							}
							if (c2 == '(')
							{
								color = BLUE_TEXT;
								function_declaration = true;
							}
						}
					}

					if (color == YELLOW_TEXT && pt_get_color(pt, f.global_char_index - 1) == BLUE_TEXT)
					{
						function_declaration = true;
					}
				}
			}

			ColorIndex* new = ci_create(color, i, f.global_char_index + i);
			if (new != NULL)
			{
				if (!ci_add_and_subtract(pt, t, ci, new, &f))
				{
					break;
				}
				i = 0;		
			}
			else
			{
				break;
			}
		}
		if (control_chars[(int) c])
		{
			function_declaration = false;
		}
		if (operator_chars[(int) c])
		{
			while (operator_chars[(int) c] && i < ci->len)
			{
				c = pt_iterate(&pi);
				i++;
			}

			ColorIndex* new = ci_create(RED_TEXT, i, f.global_char_index + i);
			if (new != NULL)
			{
				if (!ci_add_and_subtract(pt, t, ci, new, &f))
				{
					break;
				}
				i = 0;		
			}
			else
			{
				break;
			}
		}
	}
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

	char c = pt_iterate(&pi);
	int i = 0;
	bool commented = false;
	bool quote = false;

	int store;
	for (; i < ci->len; i++, c = pt_iterate(&pi))
	{
		if (c == '/')
		{
			store = i;
			c = pt_iterate(&pi);
			i++;
			if (c == '/')
			{
				commented = true;
				while (c != '\n' && c != '\0' && i < ci->len)
				{
					c = pt_iterate(&pi);
					i++;
				}
				break;
			}
		}
		else if (c == '\'' || c == '"')
		{
			quote = true;
			store = i;
			char store_char = c;
			c = pt_iterate(&pi);
			i++;

			while (c != store_char && c != '\n' && c != '\0' && i < ci->len)
			{
				c = pt_iterate(&pi);
				i++;
			}
			if (c == store_char)
			{
				c = pt_iterate(&pi);
				i++;
			}
			break;
		}
	}

	if (commented || quote)
	{
		if (store > 0)
		{
			ColorIndex* new = ci_create(WHITE_TEXT, store, f.global_char_index + store);
			if (new != NULL)
			{
				if (ci_add_and_subtract(pt, t, ci, new, &f))
				{
					pt_update_color_indices(pt, f.global_char_index - 1);
				}
			}
			else
			{
				return;
			}
		}

		ColorIndex* new = ci_create(GREEN_TEXT, i - store, f.global_char_index + (i - store));
		if (quote)
		{
			new->color = RED_TEXT;
		}
		if (new != NULL)
		{
			if (ci_add_and_subtract(pt, t, ci, new, &f))
			{
				pt_update_color_indices(pt, f.global_char_index);
			}
			return;
		}
		else
		{
			return;
		}
	}

	if (!pt_iterator_init(pt, &pi, f.global_char_index))
	{
		return;
	}

	c = pt_iterate(&pi);
	i = 0;
	for (; i < ci->len && (c == ' ' || c == '\n'); c = pt_iterate(&pi), i++) {}

	if (c == '#')
	{
		while (c != ' ' && c != '\n' && c != '\0' && i < ci->len)
		{
			c = pt_iterate(&pi);
			i++;
		}
		while (c == ' ' && i < ci->len)
		{
			c = pt_iterate(&pi);
			i++;
		}
		if (i > 0)
		{
			ColorIndex* new = ci_create(MAGENTA_TEXT, i, f.global_char_index + i);
			if (new != NULL)
			{
				if (ci_add_and_subtract(pt, t, ci, new, &f))
				{
					i = 0;

					while (c != '\n' && c != '\0' && i < ci->len)
					{
						c = pt_iterate(&pi);
						i++;
					}
					while (c == '\n' && i < ci->len)
					{
						c = pt_iterate(&pi);
						i++;
					}

					if (i > 0)
					{
						ColorIndex* new2 = ci_create(RED_TEXT, i, f.global_char_index + i);
						if (new2 != NULL)
						{
							if (ci_add_and_subtract(pt, t, ci, new2, &f))
							{
								pt_update_color_indices(pt, f.global_char_index);
							}
							return;
						}
						else
						{
							return;
						}
					}
					else
					{
						pt_update_color_indices(pt, f.global_char_index);
						return;
					}
				}
				else
				{
					return;
				}
			}
			else
			{
				return;
			}
		}
		else
		{
			ci->color = MAGENTA_TEXT;
			return;
		}
	}

	if (ci != NULL)
	{
		pt_update_color_indices_helper(pt, f.global_char_index);
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

void merge_color_indices_on_boundary(PieceTable* pt, int start_index, int end_index)
{
	if (pt == NULL || start_index < 0 || end_index < start_index)
	{
		return;
	}

	ColorIndexFinder f;
	f.contained = start_index + 1;
	f.global_char_index = -1;

	ColorIndex* ci = tree_get(pt->color_indices, &f, &ci_finder_compare_characters);
	if (ci == NULL)
	{
		return;
	}

	start_index = f.global_char_index;

	int removed_chars = 0;
	while (1)
	{
		f.contained = start_index + 1;
		f.global_char_index = -1;
		ColorIndex* ci = tree_get(pt->color_indices, &f, &ci_finder_compare_characters);
		if (ci == NULL)
		{
			break;
		}

		f.contained = start_index + 1;
		f.global_char_index = -1;
		pt->color_indices = tree_rm(pt->color_indices, &f, &ci_finder_compare_characters, NULL, &ci_update_info);

		removed_chars += ci->len;
		free(ci);
		if (start_index + removed_chars > end_index)
		{
			break;
		}
	}

	ci = ci_create(CYAN_TEXT, removed_chars, start_index + removed_chars);
	if (ci != NULL)
	{
		pt->color_indices = tree_insert(pt->color_indices, ci, &ci_compare, &ci_update_info);
	}
}

char ci_get_len(void* v)
{
	return (char) ((ColorIndex*) v)->len;
}

char ci_get_color(void* v)
{
	return (char) ((ColorIndex*) v)->color;
}
