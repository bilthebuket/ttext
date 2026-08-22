#include <stdlib.h>
#include <ncurses.h>
#include "normal_mode.h"
#include "insert_mode.h"
#include "terminal_mode.h"
#include "global.h"
#include "io_tools.h"
#include "line.h"
#include "finder.h"
#include "piece_table/undo.h"
#include "undo.h"

static bool dependent_action_chars[NUM_CHARS];

static void handle_default(EditorState* es)
{
	(void) es;
	return;
}

static void handle_h(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	if (t->x > 0)
	{
		t->x--;
		t->saved_x_index = t->x;
		check_left_update(t);
		move_cursor_to_tab(t);
	}
}

static void handle_j(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	int line_below_index = pt_get_line_index(t->pt, t->y + 1);

	if (line_below_index > 0)
	{
		int i;
		for (i = line_below_index; pt_get(t->pt, i) != '\n' && pt_get(t->pt, i) != '\0'; i++) {}
		if (i - line_below_index - 1 <= t->saved_x_index)
		{
			i -= line_below_index + 1;
		}
		else
		{
			i = t->saved_x_index;
		}
		if (i < 0)
		{
			t->x = 0;
		}
		else
		{
			t->x = i;
		}

		t->y++;
		
		check_bottom_update(t);
		check_left_update(t);

		move_cursor_to_tab(t);
	}
}

static void handle_k(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	if (t->y > 0 && pt_get_line_index(t->pt, t->y - 1) >= 0)
	{
		int line_above_index = pt_get_line_index(t->pt, t->y - 1);

		int i;
		for (i = line_above_index; pt_get(t->pt, i) != '\n' && pt_get(t->pt, i) != '\0'; i++) {}
		if (i - line_above_index - 1 <= t->saved_x_index)
		{
			i -= line_above_index + 1;
		}
		else
		{
			i = t->saved_x_index;
		}
		if (i < 0)
		{
			t->x = 0;
		}
		else
		{
			t->x = i;
		}

		t->y--;

		check_top_update(t);
		check_left_update(t);

		move_cursor_to_tab(t);
	}
}

static void handle_l(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	if (pt_get(t->pt, line_index + t->x) != '\0' && pt_get(t->pt, line_index + t->x) != '\n' && pt_get(t->pt, line_index + t->x + 1) != '\0' && pt_get(t->pt, line_index + t->x + 1) != '\n')
	{
		t->x++;
		t->saved_x_index = t->x;
		check_right_update(t);
		move_cursor_to_tab(t);
	}
}

static void handle_t(EditorState* es)
{
	print_terminal();
	move_cursor_to_terminal();
	es->mode = &terminal_mode;
}

static void handle_i(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	t->tab_num_flags &= ~CHANGES_SAVED;
	print_message("Insert Mode");

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index >= 0 && (t->tab_num_flags & PARSE_FOR_SIGNATURES))
	{
		su_prepare(es->signatures, t->pt, &(t->su), t->fname, line_index + t->x);
	}

	pt_undo_insert(t->pt);
	ci_prepare(t->pt, line_index + t->x);
	undo_insert(es, line_index + t->x);

	es->mode = &insert_mode;
}

static void handle_a(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	t->tab_num_flags &= ~CHANGES_SAVED;
	print_message("Insert Mode");
	if (pt_get(t->pt, line_index + t->x) != '\0' && pt_get(t->pt, line_index + t->x) != '\n')
	{
		t->x++;
		if (t->left_column_index + t->width < t->x)
		{
			t->left_column_index = t->x - t->width;
			print_tab(t);
		}
		move_cursor_to_tab(t);
	}
	pt_undo_insert(t->pt);
	ci_prepare(t->pt, line_index + t->x);
	undo_insert(es, line_index + t->x);

	line_index = pt_get_line_index(t->pt, t->y);
	if (line_index >= 0 && (t->tab_num_flags & PARSE_FOR_SIGNATURES))
	{
		su_prepare(es->signatures, t->pt, &(t->su), t->fname, line_index + t->x);
	}

	es->mode = &insert_mode;
}

static void handle_zero(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	t->x = 0;
	t->saved_x_index = 0;
	check_left_update(t);
	move_cursor_to_tab(t);
}

static void handle_dollar_sign(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	int len;
	for (len = 0; pt_get(t->pt, line_index + len) != '\n' && pt_get(t->pt, line_index + len) != '\0'; len++) {}
	t->x = len - 1;
	t->saved_x_index = len - 1;
	check_right_update(t);
	move_cursor_to_tab(t);
}

static void handle_o(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	int len;
	for (len = 0; pt_get(t->pt, line_index + len) != '\n' && pt_get(t->pt, line_index + len) != '\0'; len++) {}

	ci_prepare(t->pt, line_index + len);
	pt_undo_insert(t->pt);
	undo_insert(es, line_index + len);
	pt_insert(t->pt, '\n', line_index + len);
	ci_handle_insert(t->pt);
	undo_handle_insert(es);

	t->y++;
	t->x = indent_line(es, t, t->y);

	line_index = pt_get_line_index(t->pt, t->y);
	if (line_index >= 0 && (t->tab_num_flags & PARSE_FOR_SIGNATURES))
	{
		su_prepare(es->signatures, t->pt, &(t->su), t->fname, line_index + t->x);
	}


	print_tab(t);
	move_cursor_to_tab(t);
	es->mode = &insert_mode;
}

static void handle_x(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	pt_undo_insert(t->pt);
	undo_insert(es, line_index + t->x);
	ci_prepare(t->pt, line_index + t->x);

	if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		su_prepare(es->signatures, t->pt, &(t->su), t->fname, line_index + t->x);
	}

	if (pt_get(t->pt, line_index + t->x) != '\n' && pt_get(t->pt, line_index + t->x) != '\0')
	{
		t->tab_num_flags &= ~CHANGES_SAVED;
		if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
		{
			su_handle_deletion(es->signatures, t->pt, t->fname, &(t->su), line_index + t->x);
		}
		pt_rm(t->pt, line_index + t->x);
		undo_handle_delete(es);
		ci_handle_rm(t->pt);
		backup_increment_and_check(es->active_tab);

		if ((pt_get(t->pt, line_index + t->x) == '\0' || pt_get(t->pt, line_index + t->x) == '\n') && t->x > 0)
		{
			t->x--;
			t->saved_x_index = t->x;
			check_left_update(t);
			move_cursor_to_tab(t);
		}

		print_line(t, t->y);

		es->flags |= UPDATE_FINDER_FLAG;

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
	}
}

static void handle_percent_sign(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	char c = pt_get(t->pt, line_index + t->x);
	char looking_for;
	int delta;
	if (c == '(' || c == '[' || c == '{')
	{
		delta = 1;

		if (c == '(')
		{
			looking_for = ')';
		}
		else if (c == '[')
		{
			looking_for = ']';
		}
		else
		{
			looking_for = '}';
		}
	}
	else if (c == ')' || c == ']' || c== '}')
	{
		delta = -1;

		if (c == ')')
		{
			looking_for = '(';
		}
		else if (c == ']')
		{
			looking_for = '[';
		}
		else
		{
			looking_for = '{';
		}
	}
	else
	{
		return;
	}

	bool found = false;
	int counter = -1; // starting at negative one because the first character we see is the one the user pressed '%' on
			  // and we need to exclude it from the counter
	for (int y_index = t->y; y_index >= 0 && pt_get_line_index(t->pt, y_index) != -1; y_index += delta)
	{
		int line_we_are_on = pt_get_line_index(t->pt, y_index);
		int len;
		for (len = 0; pt_get(t->pt, line_we_are_on + len) != '\0' && pt_get(t->pt, line_we_are_on + len) != '\n'; len++) {}

		int x_index;
		if (y_index == t->y)
		{
			x_index = t->x;
		}
		else if (delta == 1)
		{
			x_index = 0;
		}
		else
		{
			x_index = len - 1;
		}
		for (; pt_get(t->pt, line_we_are_on + x_index) != '\0' && x_index >= 0 && x_index < len; x_index += delta)
		{
			if (pt_get(t->pt, line_we_are_on + x_index) == looking_for)
			{
				if (counter == 0)
				{
					found = true;
					t->x = x_index;
					t->y = y_index;

					if (delta == -1)
					{
						check_top_update(t);
					}
					else
					{
						check_bottom_update(t);
					}
					check_left_update(t);
					check_right_update(t);

					move_cursor_to_tab(t);
					break;
				}
				else
				{
					counter--;
				}
			}
			else if (pt_get(t->pt, line_we_are_on + x_index) == c)
			{
				counter++;
			}
		}
		if (found)
		{
			break;
		}
	}

	t->saved_x_index = t->x;
}

static void handle_n(EditorState* es)
{
	if (es->flags & UPDATE_FINDER_FLAG)
	{
		// making a copy of looking_for instead of using it directly because it gets freed
		// in finder_free. i could call free() on es->finder which wouldn't free es->finder->looking_for,
		// however that would make the code less readable
		int len = 0;
		for (; es->finder->looking_for[len] != '\0'; len++) {}
		len++;

		char* copy = malloc(sizeof(char) * len);
		for (int i = 0; i < len; i++)
		{
			copy[i] = es->finder->looking_for[i];
		}

		finder_free(es->finder);
		es->finder = finder_create(es->active_tab->pt, copy);
		es->flags &= ~UPDATE_FINDER_FLAG;
	}
	find_next(es->active_tab, es->finder);
	move_cursor_to_tab(es->active_tab);
}

static void handle_u(EditorState* es)
{
	undo_prepare_for_execute(es);
	pt_undo_execute(es->active_tab->pt);
	undo_execute(es);
	es->flags |= UPDATE_FINDER_FLAG;
	move_cursor_to_valid_coordinates(es->active_tab);
	backup_increment_and_check(es->active_tab);
	print_tab(es->active_tab);
}

static void handle_p(EditorState* es)
{
	print_pt_to_message_bar(es->active_tab->pt);
}

static void (*execute_char[NUM_CHARS])(EditorState*);

void normal_mode_create(void)
{
	for (int i = 0; i < NUM_CHARS; i++)
	{
		execute_char[i] = &handle_default;
		dependent_action_chars[i] = false;
	}
	execute_char['h'] = &handle_h;
	execute_char['j'] = &handle_j;
	execute_char['k'] = &handle_k;
	execute_char['l'] = &handle_l;
	execute_char['t'] = &handle_t;
	execute_char['i'] = &handle_i;
	execute_char['a'] = &handle_a;
	execute_char['0'] = &handle_zero;
	execute_char['$'] = &handle_dollar_sign;
	execute_char['o'] = &handle_o;
	execute_char['x'] = &handle_x;
	execute_char['%'] = &handle_percent_sign;
	execute_char['n'] = &handle_n;
	execute_char['u'] = &handle_u;
	execute_char['p'] = &handle_p;

	dependent_action_chars['d'] = true;
	dependent_action_chars['f'] = true;
	dependent_action_chars['F'] = true;
	dependent_action_chars['t'] = true;
	dependent_action_chars['T'] = true;
}

void normal_mode(EditorState* es, int ch)
{
	if (es == NULL)
	{
		return;
	}
	if (ch >= 0 && ch < NUM_CHARS)
	{
		if (es->dependent_action != '\0')
		{
			char store = es->dependent_action;
			es->dependent_action = ch;
			(*execute_char[store])(es);
			es->action_repeat = 0;
			es->dependent_action = '\0';
		}
		else if (dependent_action_chars[ch])
		{
			es->dependent_action = ch;
		}
		else
		{
			(*execute_char[ch])(es);
			es->action_repeat = 0;
			es->dependent_action = '\0';
		}
	}
}
