#include <stdlib.h>
#include <ncurses.h>
#include "linked_list.h"
#include "insert_mode.h"
#include "normal_mode/normal_mode.h"
#include "tab.h"
#include "global.h"
#include "io_tools.h"
#include "line.h"
#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"
#include "undo.h"
#include "fuzzy_find.h"

static void update_autocomplete(char** fuzzy_find, int fuzzy_find_len, DynamicArray* str, char c)
{
	if (str == NULL)
	{
		return;
	}

	if (c == BACKSPACE_KEYCODE2)
	{
		da_rm(str, str->len - 2);
	}
	else if (c == ' ' || c == '\n' || c == '\t')
	{
		for (int i = str->len - 2; i >= 0; i--)
		{
			da_rm(str, i);
		}
	}
	else
	{
		da_insert(str, c, str->len - 1);
	}

	order_by_closest_match(fuzzy_find, fuzzy_find_len, str->arr);
}

static void handle_default(EditorState* es, int ch)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}
	if (t->pt == NULL)
	{
		return;
	}

	update_autocomplete(es->fuzzy_find, es->fuzzy_find_len, t->active_string, ch);

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		su_handle_insertion(es->signatures, t->pt, t->fname, &(t->su), line_index + t->x);
	}
	pt_insert(t->pt, ch, line_index + t->x);
	undo_handle_insert(es);
	ci_handle_insert(t->pt);
	if (ch == '}')
	{
		bool indent = true;
		int j;
		for (j = 0; pt_get(t->pt, j + line_index) != '}' && pt_get(t->pt, j + line_index) != '\0'; j++)
		{
			if (pt_get(t->pt, j + line_index) != ' ')
			{
				indent = false;
				break;
			}
		}
		if (pt_get(t->pt, j + line_index) == '\0')
		{
			indent = false;
		}
		if (indent)
		{
			int amount;
			if (j >= TAB_SIZE)
			{
				amount = TAB_SIZE;
			}
			else
			{
				amount = j;
			}

			for (int i = 0; i < amount; i++)
			{
				if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
				{
					su_handle_deletion(es->signatures, t->pt, t->fname, &(t->su), line_index);
				}
				pt_rm(t->pt, line_index);
				undo_handle_delete(es);
				ci_handle_rm(t->pt);
			}
			t->x -= amount;
		}
	}

	t->x++;
	check_right_update(t);
	move_cursor_to_tab(t);

	print_line(t, t->y);
	print_autocomplete_suggestions(t, es->fuzzy_find, es->fuzzy_find_len);
}

static void handle_tab(EditorState* es, int ch)
{
	(void) ch;
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}
	if (t->pt == NULL)
	{
		return;
	}

	update_autocomplete(es->fuzzy_find, es->fuzzy_find_len, t->active_string, ch);

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	for (int i = 0; i < TAB_SIZE; i++)
	{
		if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
		{
			su_handle_insertion(es->signatures, t->pt, t->fname, &(t->su), line_index + t->x + i);
		}
		pt_insert(t->pt, ' ', line_index + t->x + i);
		undo_handle_insert(es);
		ci_handle_insert(t->pt);
	}
	t->x += TAB_SIZE;
	check_right_update(t);
	move_cursor_to_tab(t);
	print_line(t, line_index);
	print_autocomplete_suggestions(t, es->fuzzy_find, es->fuzzy_find_len);
}

static void handle_backspace(EditorState* es, int ch)
{
	(void) ch;
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}
	if (t->pt == NULL)
	{
		return;
	}

	update_autocomplete(es->fuzzy_find, es->fuzzy_find_len, t->active_string, ch);

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	if (t->x > 0)
	{
		if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
		{
			su_handle_deletion(es->signatures, t->pt, t->fname, &(t->su), line_index + t->x - 1);
		}
		pt_rm(t->pt, line_index + t->x - 1);
		undo_handle_delete(es);
		ci_handle_rm(t->pt);

		t->x--;
		check_left_update(t);
		move_cursor_to_tab(t);

		print_line(t, t->y);
	}
	else if (t->y > 0)
	{
		int line_above_index = pt_get_line_index(t->pt, t->y - 1);
		if (line_above_index >= 0)
		{
			t->y--;
			for (t->x = 0; pt_get(t->pt, line_above_index + t->x) != '\n' && pt_get(t->pt, line_above_index + t->x) != '\0'; t->x++) {}
			if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
			{
				su_handle_deletion(es->signatures, t->pt, t->fname, &(t->su), line_index - 1);
			}
			pt_rm(t->pt, line_index - 1);
			undo_handle_delete(es);
			ci_handle_rm(t->pt);

			check_left_update(t);
			check_right_update(t);
			check_top_update(t);
			move_cursor_to_tab(t);

			print_tab(t);
		}
	}

	print_autocomplete_suggestions(t, es->fuzzy_find, es->fuzzy_find_len);
}

static void handle_escape(EditorState* es, int ch)
{
	(void) ch;
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}
	if (t->pt == NULL)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	print_message("Normal Mode");
	if (t->x > 0)
	{
		t->x--;
	}
	t->saved_x_index = t->x;
	check_left_update(t);
	move_cursor_to_tab(t);
	backup_increment_and_check(es->active_tab);
	if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		su_execute(es->signatures, t->pt, &(t->su), t->fname);
	}
	int start_index;
	int end_index;
	if (ci_execute(t->pt, &start_index, &end_index))
	{
		for (int i = start_index; i <= end_index; i++)
		{
			print_line(t, i);
		}
	}
	es->mode = &normal_mode;
	es->flags |= UPDATE_FINDER_FLAG;

	da_free(t->active_string);
	t->active_string = NULL;
	for (int i = 0; i < es->fuzzy_find_len; i++)
	{
		free(es->fuzzy_find[i]);
	}
	free(es->fuzzy_find);
	es->fuzzy_find = NULL;
	es->fuzzy_find_len = 0;
	unprint_autocomplete_suggestions(t);
}

static void handle_enter(EditorState* es, int ch)
{
	(void) ch;
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}
	if (t->pt == NULL)
	{
		return;
	}

	update_autocomplete(es->fuzzy_find, es->fuzzy_find_len, t->active_string, ch);

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		su_handle_insertion(es->signatures, t->pt, t->fname, &(t->su), line_index + t->x);
	}
	pt_insert(t->pt, '\n', line_index + t->x);
	undo_handle_insert(es);
	ci_handle_insert(t->pt);

	t->y++;
	t->x = indent_line(es, t, t->y);

	check_left_update(t);
	check_bottom_update(t);
	move_cursor_to_tab(t);

	print_tab(t);
	print_autocomplete_suggestions(t, es->fuzzy_find, es->fuzzy_find_len);
}

static void control_helper(EditorState* es, int to_autocomplete)
{
	Tab* t = es->active_tab;
	if (t == NULL || to_autocomplete >= es->fuzzy_find_len)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	int store_x = t->x;

	for (int i = 0; i < t->active_string->len - 1; i++)
	{
		if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
		{
			su_handle_deletion(es->signatures, t->pt, t->fname, &(t->su), line_index + t->x - 1);
		}
		pt_rm(t->pt, line_index + t->x - 1);
		undo_handle_delete(es);
		ci_handle_rm(t->pt);

		t->x--;
	}
	for (int i = 0; es->fuzzy_find[to_autocomplete][i] != '\0'; i++)
	{
		if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
		{
			su_handle_insertion(es->signatures, t->pt, t->fname, &(t->su), line_index + t->x);
		}
		pt_insert(t->pt, es->fuzzy_find[to_autocomplete][i], line_index + t->x);
		undo_handle_insert(es);
		ci_handle_insert(t->pt);
		t->x++;
	}

	if (t->x < store_x)
	{
		check_left_update(t);
		move_cursor_to_tab(t);
	}
	else if (t->x > store_x)
	{
		check_right_update(t);
		move_cursor_to_tab(t);
	}

	print_line(t, t->y);
}

static void handle_ctrl_a(EditorState* es, int ch)
{
	(void) ch;
	control_helper(es, 4);
}

static void handle_ctrl_s(EditorState* es, int ch)
{
	(void) ch;
	control_helper(es, 3);
}

static void handle_ctrl_d(EditorState* es, int ch)
{
	(void) ch;
	control_helper(es, 2);
}

static void handle_ctrl_f(EditorState* es, int ch)
{
	(void) ch;
	control_helper(es, 1);
}

static void handle_ctrl_g(EditorState* es, int ch)
{
	(void) ch;
	control_helper(es, 0);
}

static void (*execute_char[NUM_CHARS])(EditorState*, int);

void insert_mode_create(void)
{
	for (int i = 0; i < NUM_CHARS; i++)
	{
		execute_char[i] = &handle_default;
	}
	execute_char['\t'] = &handle_tab;
	execute_char[BACKSPACE_KEYCODE2] = &handle_backspace;
	execute_char[ESCAPE_KEYCODE] = &handle_escape;
	execute_char[ENTER_KEYCODE1] = &handle_enter;
	execute_char[CTRL_G] = &handle_ctrl_g;
	execute_char[CTRL_F] = &handle_ctrl_f;
	execute_char[CTRL_D] = &handle_ctrl_d;
	execute_char[CTRL_S] = &handle_ctrl_s;
	execute_char[CTRL_A] = &handle_ctrl_a;
}

void insert_mode(EditorState* es, int ch)
{
	if (es == NULL)
	{
		return;
	}
	(*execute_char[ch])(es, ch);
}
