#include "highlight_mode.h"
#include "normal_mode/normal_mode.h"
#include "global.h"
#include "io_tools.h"

static void return_to_normal_mode(EditorState* es)
{
	es->mode = &normal_mode;
	es->action = '\0';
	es->motion = '\0';
	es->target = '\0';
	es->action_repeat = 0;
	if (es->active_tab != NULL)
	{
		es->active_tab->highlight_x = -1;
		es->active_tab->highlight_y = -1;
		print_tab(es->active_tab);
	}
	print_message("Normal Mode");
}

void highlight_mode(EditorState* es, int ch)
{
	if (es == NULL)
	{
		return;
	}


	if (es->action != '\0' || (ch >= '0' && ch <= '9') || is_motion(ch))
	{
		int store_x = -1;
		int store_y = -1;
		if (es->active_tab != NULL)
		{
			store_x = es->active_tab->x;
			store_y = es->active_tab->y;
		}
		normal_mode(es, ch);
		if (es->active_tab != NULL)
		{
			if (store_y >= 0)
			{
				if (store_y < es->active_tab->y)
				{
					for (int i = store_y; i <= es->active_tab->y; i++)
					{
						print_line(es->active_tab, i);
					}
				}
				else if (store_y > es->active_tab->y)
				{
					for (int i = es->active_tab->y; i <= store_y; i++)
					{
						print_line(es->active_tab, i);
					}
				}
				else if (store_x >= 0 && store_x != es->active_tab->x)
				{
					print_line(es->active_tab, es->active_tab->y);
				}
			}
			else if (store_x >= 0 && store_x != es->active_tab->x)
			{
				print_line(es->active_tab, es->active_tab->y);
			}
		}
	}
	else
	{
		Tab* t = es->active_tab;
		if (t == NULL)
		{
			return;
		}

		switch (ch)
		{
			case 'd':
			{
				int start_index = pt_get_line_index(t->pt, t->highlight_y);
				if (start_index < 0)
				{
					return_to_normal_mode(es);
					return;
				}

				int end_index = pt_get_line_index(t->pt, t->y);
				if (end_index < 0)
				{
					return_to_normal_mode(es);
					return;
				}

				start_index += t->highlight_x;
				end_index += t->x;

				if (start_index > end_index)
				{
					int store = start_index;
					start_index = end_index;
					end_index = store;
				}

				handle_rm_on_boundary(es, start_index, end_index);

				move_cursor_to_valid_coordinates(t);
				check_left_update(t);
				check_top_update(t);
				t->saved_x_index = t->x;
				return_to_normal_mode(es);
				break;
			}
			case '<':
			{
				int start_index;
				int end_index;
				if (t->y > t->highlight_y)
				{
					start_index = t->highlight_y;
					end_index = t->y;
				}
				else
				{
					start_index = t->y;
					end_index = t->highlight_y;
				}

				for (int i = start_index; i <= end_index; i++)
				{
					PieceIterator pi;
					int line_index = pt_get_line_index(t->pt, i);
					if (line_index < 0)
					{
						continue;
					}

					if (pt_iterator_init(t->pt, &pi, line_index))
					{
						char c = pt_iterate(&pi);
						int j = 0;
						for (; j < es->action_repeat * TAB_SIZE && c == ' '; j++, c = pt_iterate(&pi)) {}
						if (j - 1 >= line_index)
						{
							handle_rm_on_boundary(es, line_index, j - 1);
						}
					}
				}

				move_cursor_to_valid_coordinates(t);
				check_left_update(t);
				check_top_update(t);
				t->saved_x_index = t->x;
				return_to_normal_mode(es);
				break;
			}
			case '>':
			{
				int start_index;
				int end_index;
				if (t->y > t->highlight_y)
				{
					start_index = t->highlight_y;
					end_index = t->y;
				}
				else
				{
					start_index = t->y;
					end_index = t->highlight_y;
				}

				for (int i = start_index; i <= end_index; i++)
				{
					int line_index = pt_get_line_index(t->pt, i);
					if (line_index < 0)
					{
						continue;
					}
					
					for (int j = 0; j < es->action_repeat * TAB_SIZE; j++)
					{
						pt_insert(t->pt, ' ', line_index);
					}
					print_line(t, i);
				}

				t->saved_x_index = t->x;
				return_to_normal_mode(es);
				break;
			}
			case ESCAPE_KEYCODE:
			{
				return_to_normal_mode(es);
				break;
			}
		}
	}
}
