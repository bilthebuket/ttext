#include <stdlib.h>
#include <ncurses.h>
#include "LL.h"
#include "insert_mode.h"
#include "normal_mode.h"
#include "tab.h"
#include "global.h"
#include "io_tools.h"
#include "line.h"

void insert_mode(int ch)
{
	Line* line = (Line*) get_elt(active_tab->lines, active_tab->y);
	if (line == NULL)
	{
		log_error("Accessing line in tab results in NULL\n");
		return;
	}
	GapBuffer* gb = line->gb;
	if (gb == NULL)
	{
		log_error("Accessing text in line results in NULL\n");
		return;
	}

	int i;

	switch (ch)
	{
		default:
		for (i = active_tab->x; gb_get(gb, i) != '\0'; i++) {}
		if (i != LINE_SIZE - 1)
		{
			gb_goto(gb, active_tab->x);
			gb_put(gb, ch);

			if (ch == '\t')
			{
				convert_tabs_to_spaces(gb);
				active_tab->x += TAB_SIZE - 1;
			}
			if (ch == '}')
			{
				bool indent = true;
				int j;
				for (j = 0; gb_get(gb, j) != '}'; j++)
				{
					if (gb_get(gb, j) != ' ')
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
						gb_goleft(gb);
						gb_rm(gb);
					}
					active_tab->x -= amount;
				}
			}

			active_tab->x++;
			check_right_update(active_tab);
			move_cursor_to_tab(active_tab);

			update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));
			print_line(active_tab, active_tab->y);
		}
		break;

		case BACKSPACE_KEYCODE2:
		if (active_tab->x > 0)
		{
			gb_goto(gb, active_tab->x - 1);
			gb_rm(gb);

			active_tab->x--;
			check_left_update(active_tab);
			move_cursor_to_tab(active_tab);

			update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));
			print_line(active_tab, active_tab->y);
		}
		else if (active_tab->y > 0)
		{
			Line* line_above = (Line*) get_elt(active_tab->lines, active_tab->y - 1);
			if (line_above == NULL)
			{
				log_error("Accessing line in tab results in NULL\n");
				return;
			}
			char* gb_above = line_above->gb;
			if (gb_above == NULL)
			{
				log_error("Accessing text in line results in NULL\n");
				return;
			}

			int store = gb_above->num_chars - 1;
			gb_goto(gb_above, store);
			for (i = 0; i < gb->num_chars - 1; i++)
			{
				gb_put(gb_above, gb_get(gb, i));
			}

			rm(active_tab->lines, active_tab->y);
			free_line(line);
			gb_goto(gb_above, store);

			active_tab->x = store;
			active_tab->y--;

			check_left_update(active_tab);
			check_right_update(active_tab);
			check_top_update(active_tab);
			move_cursor_to_tab(active_tab);

			update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));
			print_tab(active_tab);
		}
		break;

		case ESCAPE_KEYCODE:
		print_message("Normal Mode");
		active_tab->saved_x_index = active_tab->x;
		mode = &normal_mode;
		break;

		case ENTER_KEYCODE1:
		GapBuffer* gb_new = gb_create(NULL, -1);
		gb_goto(gb, active_tab->x);
		while (gb_get(gb, active_tab->x) != '\0')
		{
			gb_put(gb_new, gb_rm(gb));
		}

		Line* l = malloc(sizeof(Line));
		if (l == NULL)
		{
			log_error("malloc failure\n");
			gb_free(gb_new);
			return;
		}

		l->gb = gb_new;
		l->color_indices = NULL;
		add(active_tab->lines, l, active_tab->y + 1);

		active_tab->y++;
		active_tab->x = indent_line(active_tab, active_tab->y);

		check_left_update(active_tab);
		check_bottom_update(active_tab);
		move_cursor_to_tab(active_tab);

 		update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));
 		update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y - 1));
		print_tab(active_tab);
		break;
	}
}
