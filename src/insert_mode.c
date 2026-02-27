#include <stdlib.h>
#include <ncurses.h>
#include "LL.h"
#include "insert_mode.h"
#include "normal_mode.h"
#include "tab.h"
#include "global.h"
#include "io_tools.h"
#include "line.h"
#include "piece_table.h"

Tab* insert_mode(Tab* t, int ch)
{
	if (t == NULL)
	{
		return NULL;
	}
	if (t->pt == NULL)
	{
		return t;
	}

	int line_index = pt_get_line_index(t->pt, t->y);
	print_info(t->pt->pieces, &print_piece);
	fprintf(stderr, "%s\n", t->pt->append);

	switch (ch)
	{
		default:
		pt_insert(t->pt, ch, line_index + t->x);
		if (ch == '}')
		{
			bool indent = true;
			int j;
			for (j = 0; pt_get(t->pt, j + line_index) != '}'; j++)
			{
				if (pt_get(t->pt, j + line_index) != ' ')
				{
					indent = false;
					break;
				}
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
					pt_rm(t->pt, line_index);
				}
				t->x -= amount;
			}
		}

		active_tab->x++;
		check_right_update(t);
		move_cursor_to_tab(t);

		pt_update_color_indices(t, line_index);
		print_line(t, t->y);
		break;

		case '\t':
		for (int i = 0; i < TAB_SIZE; i++)
		{
			pt_insert(t->pt, ' ', line_index + t->x + i);
		}
		t->x += TAB_SIZE;
		check_right_update(t);
		move_cursor_to_tab(t);
		pt_update_color_indices(t, line_index);
		print_line(t, line_index);
		break;

		case BACKSPACE_KEYCODE2:
		if (t->x > 0)
		{
			pt_rm(t->pt, line_index + t->x - 1);

			t->x--;
			check_left_update(t);
			move_cursor_to_tab(t);

			pt_update_color_indices(t, line_index);
			print_line(t, t->y);
		}
		else if (t->y > 0)
		{
			pt_rm(t->pt, line_index + t->x - 1);

			for (int i = line_index - 1; pt_get(t->pt, i) != '\n'; i--)
			{
				t->x++;
			}
			t->y--;

			check_left_update(t);
			check_right_update(t);
			check_top_update(t);
			move_cursor_to_tab(t);

			pt_update_color_indices(t, line_index);
			print_tab(t);
		}
		break;

		case ESCAPE_KEYCODE:
		print_message("Normal Mode");
		t->saved_x_index = t->x;
		mode = &normal_mode;
		break;

		case ENTER_KEYCODE1:
		pt_insert(t->pt, '\n', line_index + t->x);
		t->y++;
		t->x = indent_line(t, t->y);

		check_left_update(t);
		check_bottom_update(t);
		move_cursor_to_tab(t);

 		pt_update_color_indices(t, line_index);
 		pt_update_color_indices(t, line_index + t->x + 1);
		print_tab(t);
		break;
	}

	return t;
}
