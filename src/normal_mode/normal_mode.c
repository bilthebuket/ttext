#include <stdlib.h>
#include <ncurses.h>
#include "normal_mode/normal_mode.h"
#include "normal_mode/motions.h"
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
	if (t == NULL)
	{
		return;
	}

	Coordinate new_coords = get_target_index(es, 'h');

	if (new_coords.x >= 0 && new_coords.x != t->x)
	{
		t->x = new_coords.x;
		t->saved_x_index = t->x;
		check_left_update(t);
		move_cursor_to_tab(t);
	}
}

static void handle_j(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	Coordinate new_coords = get_target_index(es, 'j');

	if (new_coords.x < 0)
	{
		return;
	}

	if (new_coords.x < t->x)
	{
		t->x = new_coords.x;
		check_left_update(t);
	}
	else if (new_coords.x > t->x)
	{
		t->x = new_coords.x;
		check_right_update(t);
	}

	if (new_coords.y > t->y)
	{
		t->y = new_coords.y;
		check_bottom_update(t);
	}

	move_cursor_to_tab(t);
}

static void handle_k(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	Coordinate new_coords = get_target_index(es, 'k');

	if (new_coords.x < 0)
	{
		return;
	}

	if (new_coords.x < t->x)
	{
		t->x = new_coords.x;
		check_left_update(t);
	}
	else if (new_coords.x > t->x)
	{
		t->x = new_coords.x;
		check_right_update(t);
	}

	if (new_coords.y < t->y)
	{
		t->y = new_coords.y;
		check_top_update(t);
	}

	move_cursor_to_tab(t);
}

static void handle_l(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	Coordinate new_coords = get_target_index(es, 'l');

	if (new_coords.x >= 0 && new_coords.x != t->x)
	{
		t->x = new_coords.x;
		t->saved_x_index = t->x;
		check_right_update(t);
		move_cursor_to_tab(t);
	}
}

static void handle_exclamation(EditorState* es)
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
	if (t == NULL)
	{
		return;
	}

	Coordinate new_coords = get_target_index(es, '0');

	if (new_coords.x >= 0 && t->x != new_coords.x)
	{
		t->x = new_coords.x;
		t->saved_x_index = new_coords.x;
		check_left_update(t);
		move_cursor_to_tab(t);
	}
}

static void handle_dollar_sign(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	Coordinate new_coords = get_target_index(es, '$');

	if (new_coords.x > 0 && new_coords.x != t->x)
	{
		t->x = new_coords.x;
		t->saved_x_index = new_coords.x;
		check_right_update(t);
		move_cursor_to_tab(t);
	}
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

	if (pt_get(t->pt, line_index + t->x) != '\n' && pt_get(t->pt, line_index + t->x) != '\0')
	{
		PieceIterator pi;
		if (pt_iterator_init(t->pt, &pi, line_index + t->x))
		{
			char c = pt_iterate(&pi);
			int newline_index = line_index + t->x;
			while (c != '\n' && c != '\0')
			{
				c = pt_iterate(&pi);
				newline_index++;
			}

			if (es->action_repeat > newline_index - line_index)
			{
				es->action_repeat = newline_index - line_index;
			}

			int num_to_move_backwards = es->action_repeat - (newline_index - (line_index + t->x)) + 1;
			if (num_to_move_backwards < 0)
			{
				num_to_move_backwards = 0;
			}

			pt_undo_insert(t->pt);
			undo_insert(es, line_index + t->x + es->action_repeat - num_to_move_backwards);
			ci_prepare(t->pt, line_index + t->x + es->action_repeat - num_to_move_backwards);

			if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
			{
				su_prepare(es->signatures, t->pt, &(t->su), t->fname, line_index + t->x + es->action_repeat - num_to_move_backwards);
				su_handle_multiple_rm(es->signatures, t->pt, t->fname, &(t->su), es->action_repeat, line_index + t->x + es->action_repeat - num_to_move_backwards - 1);
			}

			t->tab_num_flags &= ~CHANGES_SAVED;

			if (num_to_move_backwards == 0)
			{
				pt_rm_on_boundary(t->pt, line_index + t->x, line_index + t->x + es->action_repeat - num_to_move_backwards - 1);
			}
			else
			{
				pt_rm_on_boundary(t->pt, line_index + t->x - num_to_move_backwards + 1, line_index + t->x + es->action_repeat - num_to_move_backwards - 1);
			}
			undo_handle_multiple_rm(es, es->action_repeat);
			ci_handle_multiple_rm(t->pt, es->action_repeat);

			backup_increment_and_check(es->active_tab);

			if (num_to_move_backwards > 0)
			{
				t->x -= num_to_move_backwards;
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
}

static void handle_percent_sign(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	Coordinate new_coords = get_target_index(es, '%');

	if (new_coords.x < 0)
	{
		return;
	}

	if (new_coords.x < t->x)
	{
		t->x = new_coords.x;
		t->saved_x_index = t->x;
		check_left_update(t);
	}
	else if (new_coords.x > t->x)
	{
		t->x = new_coords.x;
		t->saved_x_index = t->x;
		check_right_update(t);
	}

	if (new_coords.y < t->y)
	{
		t->y = new_coords.y;
		check_top_update(t);
	}
	else if (new_coords.y > t->y)
	{
		t->y = new_coords.y;
		check_bottom_update(t);
	}

	move_cursor_to_tab(t);

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
	for (int i = 0; i < es->action_repeat; i++)
	{
		find_next(es->active_tab, es->finder);
	}
	move_cursor_to_tab(es->active_tab);
}

static void handle_u(EditorState* es)
{
	for (int i = 0; i < es->action_repeat; i++)
	{
		undo_prepare_for_execute(es);
		pt_undo_execute(es->active_tab->pt);
		undo_execute(es);
	}
	es->flags |= UPDATE_FINDER_FLAG;
	move_cursor_to_valid_coordinates(es->active_tab);
	backup_increment_and_check(es->active_tab);
	print_tab(es->active_tab);
}

static void handle_p(EditorState* es)
{
	print_pt_to_message_bar(es->active_tab->pt);
}

static void handle_escape(EditorState* es)
{
	es->action_repeat = 0;
	es->dependent_action = '\0';
}

static void ftFT_helper(EditorState* es, char c)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	Coordinate new_coords = get_target_index(es, c);

	if (new_coords.x >= 0)
	{
		if (new_coords.x > t->x)
		{
			t->x = new_coords.x;
			t->saved_x_index = t->x;
			check_right_update(t);
		}
		else if (new_coords.x < t->x)
		{
			t->x = new_coords.x;
			t->saved_x_index = t->x;
			check_left_update(t);
		}
	}

	move_cursor_to_tab(t);
}

static void handle_f(EditorState* es)
{
	ftFT_helper(es, 'f');
}

static void handle_t(EditorState* es)
{
	ftFT_helper(es, 't');
}

static void handle_F(EditorState* es)
{
	ftFT_helper(es, 'F');
}

static void handle_T(EditorState* es)
{
	ftFT_helper(es, 'T');
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
	execute_char['!'] = &handle_exclamation;
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
	execute_char[ESCAPE_KEYCODE] = &handle_escape;
	execute_char['f'] = &handle_f;
	execute_char['t'] = &handle_t;
	execute_char['F'] = &handle_F;
	execute_char['T'] = &handle_T;

	dependent_action_chars['d'] = true;
	dependent_action_chars['f'] = true;
	dependent_action_chars['F'] = true;
	dependent_action_chars['t'] = true;
	dependent_action_chars['T'] = true;

	initialize_normal_mode_motions();
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
			if (es->action_repeat == 0)
			{
				es->action_repeat = 1;
			}
			(*execute_char[(int) store])(es);
			es->action_repeat = 0;
			es->dependent_action = '\0';
		}
		else if (dependent_action_chars[ch])
		{
			es->dependent_action = ch;
		}
		else if ((ch >= '1' && ch <= '9') || (ch == '0' && es->action_repeat != 0))
		{
			int num = ch - '0';
			es->action_repeat *= 10;
			es->action_repeat += num;
		}
		else
		{
			if (es->action_repeat == 0)
			{
				es->action_repeat = 1;
			}
			(*execute_char[ch])(es);
			es->action_repeat = 0;
			es->dependent_action = '\0';
		}
	}
}
