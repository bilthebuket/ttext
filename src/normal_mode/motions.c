#include "piece_table/piece_table.h"
#include "normal_mode/motions.h"

Coordinate (*do_motion[NUM_CHARS])(EditorState*);

static Coordinate handle_h(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL || t->pt == NULL)
	{
		return;
	}

	Coordinate r;
	r.y = t->y;
	r.x = t->x;

	int line_index = pt_get_line_index(t->pt, r.y);
	if (line_index < 0)
	{
		return;
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
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
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
		return;
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
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return;
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
	if ((int) motion < 0 || (int) motion >= NUM_CHARS || do_motion[(int) motion] == NULL)
	{
		return -1;
	}

	return (*(do_motion[(int) motion]))(es);
}
