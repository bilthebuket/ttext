#include <stdlib.h>
#include <ncurses.h>
#include "LL.h"
#include "insert_mode.h"
#include "normal_mode.h"
#include "tab.h"
#include "global.h"
#include "io_tools.h"

void insert_mode(int ch)
{
	char* line = (char*) get_elt(active_tab->lines, active_tab->y);

	switch (ch)
	{
		default:
		int i;
		for (i = active_tab->x; line[i] != '\0'; i++) {}
		if (i != LINE_SIZE - 1)
		{
			for (; i >= active_tab->x; i--)
			{
				line[i + 1] = line[i];
			}
			line[active_tab->x] = ch;

			active_tab->x++;
			move(active_tab->y, active_tab->x);

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
			move(active_tab->y, active_tab->x);

			print_line(active_tab, active_tab->y);
		}
		else if (line[0] == '\0')
		{
			free(rm(active_tab->lines, active_tab->y));
		}
		break;

		case ESCAPE_KEYCODE:
		print_message("Normal Mode");
		mode = &normal_mode;
		break;
	}
}
