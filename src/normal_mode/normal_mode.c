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
#include "highlight_mode.h"
#include "fuzzy_find.h"

#define ARBITRARY_SIZE 50

static bool action_needs_motion[NUM_CHARS];
static bool motion_needs_target[NUM_CHARS];
static bool action_needs_target[NUM_CHARS];
static void (*execute_char[NUM_CHARS])(EditorState*);

static void handle_default(EditorState* es)
{
	(void) es;
	return;
}

static void motion_helper(EditorState* es, bool update_saved_x)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	Coordinate new_coords = get_target_index(es, es->action);

	if (new_coords.x >= 0)
	{
		if (new_coords.x > t->x)
		{
			t->x = new_coords.x;
			if (update_saved_x)
			{
				t->saved_x_index = t->x;
			}
			check_right_update(t);
		}
		else if (new_coords.x < t->x)
		{
			t->x = new_coords.x;
			if (update_saved_x)
			{
				t->saved_x_index = t->x;
			}
			check_left_update(t);
		}
	}
	if (new_coords.y >= 0)
	{
		if (new_coords.y > t->y)
		{
			t->y = new_coords.y;
			check_bottom_update(t);
		}
		else if (new_coords.y < t->y)
		{
			t->y = new_coords.y;
			check_top_update(t);
		}
	}

	move_cursor_to_tab(t);
}

static void motion_helper_dont_update_saved_x(EditorState* es)
{
	motion_helper(es, false);
}

static void motion_helper_update_saved_x(EditorState* es)
{
	motion_helper(es, true);
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
	es->flags |= UPDATE_FINDER_FLAG;
	print_message("Insert Mode");

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index >= 0 && (t->tab_num_flags & PARSE_FOR_SIGNATURES))
	{
		su_prepare(es->signatures, t->pt, &(t->su), t->fname, line_index + t->x);
	}

	pt_undo_insert(t->pt);
	ci_prepare(t->pt, line_index + t->x);
	undo_insert(es, line_index + t->x);

	es->fuzzy_find = find_strings_to_autocomplete(es, &es->fuzzy_find_len);
	t->active_string = da_create(ARBITRARY_SIZE);
	da_insert(t->active_string, '\0', 0);
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
	es->flags |= UPDATE_FINDER_FLAG;
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

	es->fuzzy_find = find_strings_to_autocomplete(es, &es->fuzzy_find_len);
	t->active_string = da_create(ARBITRARY_SIZE);
	da_insert(t->active_string, '\0', 0);
	es->mode = &insert_mode;
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
	t->tab_num_flags &= ~CHANGES_SAVED;
	es->flags |= UPDATE_FINDER_FLAG;
	print_message("Insert Mode");

	es->fuzzy_find = find_strings_to_autocomplete(es, &es->fuzzy_find_len);
	t->active_string = da_create(ARBITRARY_SIZE);
	da_insert(t->active_string, '\0', 0);
	es->mode = &insert_mode;
}

void handle_rm_on_boundary(EditorState* es, int start_index, int end_index)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	pt_undo_insert(t->pt);
	undo_insert(es, end_index + 1);
	ci_prepare(t->pt, end_index + 1);

	if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		su_prepare(es->signatures, t->pt, &(t->su), t->fname, end_index + 1);
		su_handle_multiple_rm(es->signatures, t->pt, t->fname, &(t->su), end_index - start_index + 1, end_index);
	}

	t->tab_num_flags &= ~CHANGES_SAVED;

	pt_rm_on_boundary(t->pt, start_index, end_index);
	undo_handle_multiple_rm(es, end_index - start_index + 1);
	ci_handle_multiple_rm(t->pt, end_index - start_index + 1);

	backup_increment_and_check(es->active_tab);

	es->flags |= UPDATE_FINDER_FLAG;

	if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		su_execute(es->signatures, t->pt, &(t->su), t->fname);
	}
	ci_execute(t->pt, &start_index, &end_index);
	print_tab(t);
}

static Coordinate get_bounds_action_motion(EditorState* es, int* start_index, int* end_index)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return (Coordinate) {.x = -1, .y = -1, .x2 = -1, .y2 = -1};
	}

	Coordinate bounds = get_target_index(es, es->motion);

	if (bounds.x < 0)
	{
		*start_index = -1;
		*end_index = -1;
		return (Coordinate) {.x = -1, .y = -1, .x2 = -1, .y2 = -1};
	}

	if (bounds.x2 < 0)
	{
		bounds.x2 = t->x;
		bounds.y2 = t->y;
	}

	if (bounds.x2 < bounds.x)
	{
		int temp = bounds.x;
		bounds.x = bounds.x2;
		bounds.x2 = temp;
	}
	if (bounds.y2 < bounds.y)
	{
		int temp = bounds.y;
		bounds.y = bounds.y2;
		bounds.y2 = temp;
	}

	*start_index = pt_get_line_index(t->pt, bounds.y);
	if (*start_index < 0)
	{
		*start_index = -1;
		*end_index = -1;
		return (Coordinate) {.x = -1, .y = -1, .x2 = -1, .y2 = -1};
	}
	*start_index += bounds.x;

	*end_index = pt_get_line_index(t->pt, bounds.y2);
	if (*end_index < 0)
	{
		*start_index = -1;
		*end_index = -1;
		return (Coordinate) {.x = -1, .y = -1, .x2 = -1, .y2 = -1};
	}
	*end_index += bounds.x2;

	return bounds;
}

static void handle_d(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	int start_index;
	int end_index;
	Coordinate to_delete = get_bounds_action_motion(es, &start_index, &end_index);
	if (start_index < 0 || end_index < 0)
	{
		return;
	}

	handle_rm_on_boundary(es, start_index, end_index);
	if (to_delete.x - 1 >= 0)
	{
		t->x = to_delete.x - 1;
	}
	else
	{
		t->x = 0;
	}
	t->y = to_delete.y;

	t->saved_x_index = t->x;
	check_left_update(t);
	check_top_update(t);
	move_cursor_to_tab(t);
}

static void handle_y(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	int start_index;
	int end_index;
	get_bounds_action_motion(es, &start_index, &end_index);
	if (start_index < 0 || end_index < 0)
	{
		return;
	}

	char* to_copy = malloc(sizeof(char) * (end_index - start_index + 2));
	if (to_copy == NULL)
	{
		return;
	}

	PieceIterator pi;
	if (pt_iterator_init(t->pt, &pi, start_index))
	{
		char c = pt_iterate(&pi);
		int i = start_index;
		for (; i <= end_index; i++, c = pt_iterate(&pi))
		{
			to_copy[i - start_index] = c;
		}
		to_copy[i - start_index] = '\0';
		clipboard_insert(es->clipboard, to_copy);
	}
	else
	{
		free(to_copy);
	}
}

static void handle_Y(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	int start_index;
	int end_index;
	Coordinate to_delete = get_bounds_action_motion(es, &start_index, &end_index);
	if (start_index < 0 || end_index < 0)
	{
		return;
	}

	char* to_copy = malloc(sizeof(char) * (end_index - start_index + 2));
	if (to_copy == NULL)
	{
		return;
	}

	PieceIterator pi;
	if (pt_iterator_init(t->pt, &pi, start_index))
	{
		char c = pt_iterate(&pi);
		int i = start_index;
		for (; i <= end_index; i++, c = pt_iterate(&pi))
		{
			to_copy[i - start_index] = c;
		}
		to_copy[i - start_index] = '\0';
		clipboard_insert(es->clipboard, to_copy);

		handle_rm_on_boundary(es, start_index, end_index);

		if (to_delete.x - 1 >= 0)
		{
			t->x = to_delete.x - 1;
		}
		else
		{
			t->x = 0;
		}
		t->y = to_delete.y;

		move_cursor_to_valid_coordinates(t);
		check_left_update(t);
		check_top_update(t);
		t->saved_x_index = t->x;
	}
	else
	{
		free(to_copy);
		return;
	}
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

			int num_to_move_backwards;
			if (es->action_repeat >= newline_index - line_index)
			{
				es->action_repeat = newline_index - line_index;
				num_to_move_backwards = t->x;
			}
			else
			{
				num_to_move_backwards = es->action_repeat - (newline_index - (line_index + t->x)) + 1;
				if (num_to_move_backwards < 0)
				{
					num_to_move_backwards = 0;
				}
			}

			if (num_to_move_backwards == 0)
			{
				handle_rm_on_boundary(es, line_index + t->x, line_index + t->x + es->action_repeat - num_to_move_backwards - 1);
			}
			else
			{
				handle_rm_on_boundary(es, line_index + t->x - num_to_move_backwards + 1, line_index + t->x + es->action_repeat - num_to_move_backwards);
			}

			if (num_to_move_backwards > 0)
			{
				t->x -= num_to_move_backwards;
				t->saved_x_index = t->x;
				check_left_update(t);
				move_cursor_to_tab(t);
			}
		}
	}
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
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	for (int i = 0; i < es->action_repeat; i++)
	{
		undo_prepare_for_execute(es);
		pt_undo_execute(t->pt);
		undo_execute(es);
	}
	es->flags |= UPDATE_FINDER_FLAG;
	t->tab_num_flags &= ~CHANGES_SAVED;
	move_cursor_to_valid_coordinates(t);
	t->saved_x_index = t->x;
	backup_increment_and_check(t);
	print_tab(t);
}

static void handle_U(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	for (int i = 0; i < es->action_repeat; i++)
	{
		redo_prepare_for_execute(es);
		pt_redo_execute(t->pt);
		redo_execute(es);
	}
	es->flags |= UPDATE_FINDER_FLAG;
	t->tab_num_flags &= ~CHANGES_SAVED;
	move_cursor_to_valid_coordinates(t);
	t->saved_x_index = t->x;
	backup_increment_and_check(t);
	print_tab(t);
}

static void handle_p(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	int target_clipboard = es->target - '0';
	if (target_clipboard == 0)
	{
		target_clipboard = 9;
	}
	else
	{
		target_clipboard--;
	}

	if (!(target_clipboard >= 0 && target_clipboard <= 9))
	{
		return;
	}

	char* to_add = ll_get_elt(es->clipboard, target_clipboard);
	if (to_add == NULL)
	{
		return;
	}

	int len = 0;
	for (; to_add[len] != '\0'; len++) {}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
	}

	if (!(t->x == 0 && ((pt_get(t->pt, line_index) == '\n') || pt_get(t->pt, line_index) == '\0')))
	{
		t->x++;
	}

	t->tab_num_flags &= ~CHANGES_SAVED;
	es->flags |= UPDATE_FINDER_FLAG;

	if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		su_prepare(es->signatures, t->pt, &(t->su), t->fname, line_index + t->x);
	}

	pt_undo_insert(t->pt);
	ci_prepare(t->pt, line_index + t->x);
	undo_insert(es, line_index + t->x);

	pt_handle_multiple_insert(t->pt, to_add, line_index + t->x);
	for (int i = 0; i < len; i++, t->x++)
	{
		ci_handle_insert(t->pt);
		undo_handle_insert(es);
		if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
		{
			su_handle_insertion(es->signatures, t->pt, t->fname, &(t->su), line_index + t->x + i);
		}
	}

	move_cursor_to_valid_coordinates(t);
	t->saved_x_index = t->x;
	check_left_update(t);
	check_right_update(t);
	backup_increment_and_check(es->active_tab);
	if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		su_execute(es->signatures, t->pt, &(t->su), t->fname);
	}
	int start_index;
	int end_index;
	ci_execute(t->pt, &start_index, &end_index);
	print_tab(t);
}

static void handle_escape(EditorState* es)
{
	es->action_repeat = 0;
	es->action = '\0';
	es->motion = '\0';
	es->target = '\0';
}

static void handle_v(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	t->highlight_x = t->x;
	t->highlight_y = t->y;
	print_message("Highlight Mode");
	es->mode = &highlight_mode;
}

bool is_motion(char c)
{
	if ((int) c >= 0)
	{
		return execute_char[(int) c] == &motion_helper_update_saved_x || execute_char[(int) c] == &motion_helper_dont_update_saved_x;
	}
	return false;
}

void normal_mode_create(void)
{
	for (int i = 0; i < NUM_CHARS; i++)
	{
		execute_char[i] = &handle_default;
		action_needs_motion[i] = false;
		motion_needs_target[i] = false;
		action_needs_target[i] = false;
	}

	execute_char['h'] = &motion_helper_update_saved_x;
	execute_char['j'] = &motion_helper_dont_update_saved_x;
	execute_char['k'] = &motion_helper_dont_update_saved_x;
	execute_char['l'] = &motion_helper_update_saved_x;
	execute_char['!'] = &handle_exclamation;
	execute_char['i'] = &handle_i;
	execute_char['a'] = &handle_a;
	execute_char['0'] = &motion_helper_update_saved_x;
	execute_char['$'] = &motion_helper_update_saved_x;
	execute_char['o'] = &handle_o;
	execute_char['x'] = &handle_x;
	execute_char['%'] = &motion_helper_update_saved_x;
	execute_char['n'] = &handle_n;
	execute_char['u'] = &handle_u;
	execute_char['U'] = &handle_U;
	execute_char['p'] = &handle_p;
	execute_char[ESCAPE_KEYCODE] = &handle_escape;
	execute_char['f'] = &motion_helper_update_saved_x;
	execute_char['t'] = &motion_helper_update_saved_x;
	execute_char['F'] = &motion_helper_update_saved_x;
	execute_char['T'] = &motion_helper_update_saved_x;
	execute_char['d'] = &handle_d;
	execute_char['w'] = &motion_helper_update_saved_x;
	execute_char['v'] = &handle_v;
	execute_char['y'] = &handle_y;
	execute_char['Y'] = &handle_Y;

	action_needs_motion['d'] = true;
	action_needs_motion['y'] = true;
	action_needs_motion['Y'] = true;

	motion_needs_target['f'] = true;
	motion_needs_target['t'] = true;
	motion_needs_target['F'] = true;
	motion_needs_target['T'] = true;
	motion_needs_target['i'] = true;

	action_needs_target['f'] = true;
	action_needs_target['t'] = true;
	action_needs_target['F'] = true;
	action_needs_target['T'] = true;
	action_needs_target['p'] = true;

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
		if (es->action == '\0')
		{
			if ((ch >= '1' && ch <= '9') || (ch == '0' && es->action_repeat != 0))
			{
				int num = ch - '0';
				es->action_repeat *= 10;
				es->action_repeat += num;
			}
			else
			{
				es->action = ch;
				if (!action_needs_target[ch] && !action_needs_motion[ch])
				{
					if (es->action_repeat == 0)
					{
						es->action_repeat = 1;
					}
					(*execute_char[(int) es->action])(es);
					es->action_repeat = 0;
					es->action = '\0';
				}
			}
		}
		else if (es->motion == '\0')
		{
			if (action_needs_target[(int) es->action])
			{
				if (es->action_repeat == 0)
				{
					es->action_repeat = 1;
				}
				es->target = ch;
				(*execute_char[(int) es->action])(es);
				es->action_repeat = 0;
				es->action = '\0';
				es->target = '\0';
			}
			else
			{
				es->motion = ch;
				if (!motion_needs_target[(int) es->motion])
				{
					if (es->action_repeat == 0)
					{
						es->action_repeat = 1;
					}
					(*execute_char[(int) es->action])(es);
					es->action_repeat = 0;
					es->action = '\0';
					es->motion = '\0';
				}
			}
		}
		else
		{
			if (es->action_repeat == 0)
			{
				es->action_repeat = 1;
			}
			es->target = ch;
			(*execute_char[(int) es->action])(es);
			es->action_repeat = 0;
			es->action = '\0';
			es->motion = '\0';
			es->target = '\0';
		}
	}
}
