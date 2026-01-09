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
	char* line = ((Line*) get_elt(active_tab->lines, active_tab->y))->text;
	int i;

	switch (ch)
	{
		default:
		for (i = active_tab->x; line[i] != '\0'; i++) {}
		if (i != LINE_SIZE - 1)
		{
			for (; i >= active_tab->x; i--)
			{
				line[i + 1] = line[i];
			}
			line[active_tab->x] = ch;

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
			for (int i = active_tab->x - 1; line[i] != '\0'; i++)
			{
				line[i] = line[i + 1];
			}

			active_tab->x--;
			check_left_update(active_tab);
			move_cursor_to_tab(active_tab);

			update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));
			print_line(active_tab, active_tab->y);
		}
		else if (active_tab->y > 0)
		{
			char* line_above = ((Line*) get_elt(active_tab->lines, active_tab->y - 1))->text;

			for (i = 0; line_above[i] != '\0'; i++) {}
			int len = i;
			for (; line[i - len] != '\0' && i < LINE_SIZE; i++)
			{
				line_above[i] = line[i - len];
			}
			if (line[i - len] != '\0')
			{
				line[len] = '\0';
				print_message("Operation would cause a line to exceed the maximum line size");
			}
			else
			{
				rm(active_tab->lines, active_tab->y);
				line_above[i] = '\0';
				free(line);

				active_tab->x = i;
				active_tab->y--;

				check_left_update(active_tab);
				check_right_update(active_tab);
				check_top_update(active_tab);
				move_cursor_to_tab(active_tab);

				update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));
				print_tab(active_tab);
			}
		}
		break;

		case ESCAPE_KEYCODE:
		print_message("Normal Mode");
		mode = &normal_mode;
		break;

		case ENTER_KEYCODE1:
		char* buf = malloc(sizeof(char) * LINE_SIZE);
		for (i = active_tab->x; line[i] != '\0'; i++)
		{
			buf[i - active_tab->x] = line[i];
		}
		line[active_tab->x] = '\0';
		buf[i - active_tab->x] = '\0';

		Line* l = malloc(sizeof(Line));
		l->text = buf;
		add(active_tab->lines, l, active_tab->y + 1);

		active_tab->x = 0;
		active_tab->y++;

		check_left_update(active_tab);
		check_bottom_update(active_tab);
		move_cursor_to_tab(active_tab);

 		update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));
 		update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y - 1));
		print_tab(active_tab);
		break;
	}
}
