#include <ncurses.h>
#include "normal_mode.h"
#include "insert_mode.h"
#include "terminal_mode.h"
#include "global.h"
#include "io_tools.h"
#include "line.h"

void normal_mode(int ch)
{
	switch (ch)
	{
		case 'h':
		if (active_tab->x > 0)
		{
			active_tab->x--;
			check_left_update(active_tab);
			move_cursor_to_tab(active_tab);
		}
		break;

		case 'j':
		if (active_tab->y < active_tab->lines->size - 1)
		{
			char* line = ((Line*) get_elt(active_tab->lines, active_tab->y + 1))->text;
			int i;
			for (i = 0; line[i] != '\0' && i < active_tab->x; i++) {}
			if (line[i] == '\0')
			{
				if (i == 0)
				{
					active_tab->x = 0;
				}
				else
				{
					active_tab->x = i - 1;
				}
			}

			active_tab->y++;
			
			check_bottom_update(active_tab);
			check_left_update(active_tab);

			move_cursor_to_tab(active_tab);
		}
		break;

		case 'k':
		if (active_tab->y > 0)
		{
			char* line = ((Line*) get_elt(active_tab->lines, active_tab->y - 1))->text;
			int i;
			for (i = 0; line[i] != '\0' && i < active_tab->x; i++) {}
			if (line[i] == '\0')
			{
				if (i == 0)
				{
					active_tab->x = 0;
				}
				else
				{
					active_tab->x = i - 1;
				}
			}

			active_tab->y--;

			check_top_update(active_tab);
			check_left_update(active_tab);

			move_cursor_to_tab(active_tab);
		}
		break;

		case 'l':
		if (((Line*) get_elt(active_tab->lines, active_tab->y))->text[active_tab->x + 1] != '\0')
		{
			active_tab->x++;
			check_right_update(active_tab);
			move_cursor_to_tab(active_tab);
		}
		break;

		case 't':
		print_tab(terminal);
		move_cursor_to_tab(terminal);
		mode = &terminal_mode;
		break;

		case 'i':
		active_tab->z_index_changes_saved &= ~CHANGES_SAVED;
		print_message("Insert Mode");
		mode = &insert_mode;
		break;

		case 'a':
		active_tab->z_index_changes_saved &= ~CHANGES_SAVED;
		print_message("Insert Mode");
		active_tab->x++;
		if (active_tab->left_column_index + active_tab->width < active_tab->x)
		{
			active_tab->left_column_index = active_tab->x - active_tab->width;
			print_tab(active_tab);
		}
		move_cursor_to_tab(active_tab);
		mode = &insert_mode;
		break;

		case '0':
		active_tab->x = 0;
		check_left_update(active_tab);
		move_cursor_to_tab(active_tab);
		break;

		case '$':
		char* line = ((Line*) get_elt(active_tab->lines, active_tab->y))->text;
		int len = 0;
		for (; line[len] != '\0'; len++) {}
		active_tab->x = len - 1;
		check_right_update(active_tab);
		move_cursor_to_tab(active_tab);
		break;
	}
}
