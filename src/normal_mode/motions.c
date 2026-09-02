#include <stddef.h>
#include "piece_table/piece_table.h"
#include "normal_mode/motions.h"
#include "global.h"
#include "tab.h"

Coordinate (*do_motion[NUM_CHARS])(EditorState*);

static Coordinate handle_h(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	Coordinate r;
	r.y = t->y;
	r.x = t->x;

	int line_index = pt_get_line_index(t->pt, r.y);
	if (line_index < 0)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	if (r.x > 0)
	{
		if (r.x - es->action_repeat < 0)
		{
			r.x = 0;
		}
		else
		{
			r.x -= es->action_repeat;
		}
	}

	return r;
}

static Coordinate handle_j(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL || t->pt->pieces == NULL || t->pt->pieces->elt == NULL)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	Coordinate r;
	r.x = t->x;
	r.y = t->y;

	int num_lines_in_document = ((Piece*) t->pt->pieces->elt)->lines_contained;
	if (r.y + es->action_repeat > num_lines_in_document)
	{
		es->action_repeat = num_lines_in_document - r.y;
	}
	int line_below_index = pt_get_line_index(t->pt, r.y + es->action_repeat);

	// if we are jumping to the last line and its empty then pt_iterator_init will fail, so we need to handle that
	PieceIterator pi;
	if (line_below_index > 0 && (pt_iterator_init(t->pt, &pi, line_below_index) || ((Piece*) t->pt->pieces->elt)->chars_contained == line_below_index))
	{
		int i = line_below_index;
		if (((Piece*) t->pt->pieces->elt)->chars_contained != line_below_index)
		{
			char c = pt_iterate(&pi);
			for (; c != '\n' && c != '\0'; c = pt_iterate(&pi), i++) {}
		}

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
			r.x = 0;
		}
		else
		{
			r.x = i;
		}

		r.y += es->action_repeat;
	}

	return r;
}

static Coordinate handle_k(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	if (t->y - es->action_repeat < 0)
	{
		es->action_repeat = t->y;
	}

	Coordinate r;
	r.x = t->x;
	r.y = t->y;

	int line_above_index = pt_get_line_index(t->pt, r.y - es->action_repeat);
	PieceIterator pi;

	if (r.y > 0 && line_above_index >= 0 && pt_iterator_init(t->pt, &pi, line_above_index))
	{
		int i = line_above_index;
		char c = pt_iterate(&pi);
		for (; c != '\n' && c != '\0'; c = pt_iterate(&pi), i++) {}
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
			r.x = 0;
		}
		else
		{
			r.x = i;
		}

		r.y -= es->action_repeat;
	}

	return r;
}

static Coordinate handle_l(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	Coordinate r;
	r.y = t->y;
	r.x = t->x;

	PieceIterator pi;
	if (pt_iterator_init(t->pt, &pi, line_index + r.x))
	{
		char c = pt_iterate(&pi);
		int i = 0;
		for (; c != '\0' && c != '\n' && i < es->action_repeat; i++, c = pt_iterate(&pi), r.x++) {}

		if (i > 0)
		{
			if (c == '\n' || c == '\0')
			{
				r.x--;
			}
		}
	}

	return r;
}

static Coordinate handle_dollar_sign(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	Coordinate r;
	r.x = t->x;
	r.y = t->y;

	PieceIterator pi;
	if (pt_iterator_init(t->pt, &pi, line_index))
	{
		int len = 0;
		char c = pt_iterate(&pi);
		for (; c != '\n' && c != '\0'; len++, c = pt_iterate(&pi)) {}
		r.x = len - 1;
	}

	return r;
}

static Coordinate handle_zero(EditorState* es)
{
	if (es->active_tab == NULL)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	return (Coordinate) {.x = 0, .y = es->active_tab->y};
}

static Coordinate handle_percent_sign(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return (Coordinate) {.x = -1, .y = -1};
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
		return (Coordinate) {.x = -1, .y = -1};
	}

	Coordinate r;
	r.x = t->x;
	r.y = t->y;

	bool found = false;
	int counter = -1; // starting at negative one because the first character we see is the one the user pressed '%' on
			  // and we need to exclude it from the counter
	for (int y_index = r.y; y_index >= 0 && pt_get_line_index(t->pt, y_index) != -1; y_index += delta)
	{
		int line_we_are_on = pt_get_line_index(t->pt, y_index);
		int len = 0;
		PieceIterator pi;
		if (pt_iterator_init(t->pt, &pi, line_we_are_on))
		{
			char c2 = pt_iterate(&pi);
			for (len = 0; c2 != '\0' && c2 != '\n'; c2 = pt_iterate(&pi), len++) {}
		}

		int x_index;
		if (y_index == r.y)
		{
			x_index = r.x;
		}
		else if (delta == 1)
		{
			x_index = 0;
		}
		else
		{
			x_index = len - 1;
		}
		
		if (pt_iterator_init(t->pt, &pi, line_we_are_on + x_index))
		{
			char c2;
			if (delta == 1)
			{
				c2 = pt_iterate(&pi);
			}
			else
			{
				c2 = pt_iterate_backwards(&pi);
			}

			for (; c2 != '\0' && x_index >= 0 && x_index < len; x_index += delta)
			{
				if (c2 == looking_for)
				{
					if (counter == 0)
					{
						found = true;
						r.x = x_index;
						r.y = y_index;
						break;
					}
					else
					{
						counter--;
					}
				}
				else if (c2 == c)
				{
					counter++;
				}

				if (delta == 1)
				{
					c2 = pt_iterate(&pi);
				}
				else
				{
					c2 = pt_iterate_backwards(&pi);
				}
			}
		}
		if (found)
		{
			break;
		}
	}

	return r;
}

// final_offset -> the final result will be adjusted in the direction of delta by this amount
static Coordinate character_finder_helper(EditorState* es, int delta, int final_offset)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	Coordinate r;
	r.x = t->x;
	r.y = t->y;

	PieceIterator pi;
	if (pt_iterator_init(t->pt, &pi, line_index + t->x))
	{
		char c;
		if (delta == 1)
		{
			c = pt_iterate(&pi);
		}
		else
		{
			c = pt_iterate_backwards(&pi);
		}

		int x = r.x;
		while (es->action_repeat > 0)
		{
			while (c != '\n' && c != '\0' && c != es->dependent_action)
			{
				if (delta == 1)
				{
					c = pt_iterate(&pi);
					x++;
				}
				else
				{
					c = pt_iterate_backwards(&pi);
					x--;
				}
			}
			if (c == es->dependent_action)
			{
				es->action_repeat--;
				r.x = x;
				if (delta == 1)
				{
					c = pt_iterate(&pi);
					x++;
				}
				else
				{
					c = pt_iterate_backwards(&pi);
					x--;
				}
			}
			else
			{
				break;
			}
		}
	}

	if (r.x != t->x)
	{
		r.x += delta * final_offset;
	}

	return r;
}

static Coordinate handle_f(EditorState* es)
{
	return character_finder_helper(es, 1, 0);
}

static Coordinate handle_t(EditorState* es)
{
	return character_finder_helper(es, 1, -1);
}

static Coordinate handle_F(EditorState* es)
{
	return character_finder_helper(es, -1, 0);
}

static Coordinate handle_T(EditorState* es)
{
	return character_finder_helper(es, -1, -1);
}

void initialize_normal_mode_motions(void)
{
	for (int i = 0; i < NUM_CHARS; i++)
	{
		do_motion[i] = NULL;
	}

	do_motion['f'] = &handle_f;
	do_motion['t'] = &handle_t;
	do_motion['F'] = &handle_F;
	do_motion['T'] = &handle_T;
	do_motion['h'] = &handle_h;
	do_motion['j'] = &handle_j;
	do_motion['k'] = &handle_k;
	do_motion['l'] = &handle_l;
	do_motion['$'] = &handle_dollar_sign;
	do_motion['0'] = &handle_zero;
	do_motion['%'] = &handle_percent_sign;
}

Coordinate get_target_index(EditorState* es, char motion)
{
	if ((int) motion < 0 || do_motion[(int) motion] == NULL)
	{
		return (Coordinate) {.x = -1, .y = -1};
	}

	return (*(do_motion[(int) motion]))(es);
}
