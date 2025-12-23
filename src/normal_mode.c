#include <ncurses.h>
#include "normal_mode.h"
#include "insert_mode.h"
#include "global.h"
#include "io_tools.h"

void normal_mode(int ch)
{
	switch (ch)
	{
		case 'h':
		if (active_tab->x > 0)
		{
			active_tab->x--;
			move(active_tab->y, active_tab->x);
		}
		break;

		case 'j':
		if (active_tab->y < active_tab->lines->size - 1)
		{
			char* line = (char*) get_elt(active_tab->lines, active_tab->y + 1);
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
			move(active_tab->y, active_tab->x);
		}
		break;

		case 'k':
		if (active_tab->y > 0)
		{
			char* line = (char*) get_elt(active_tab->lines, active_tab->y - 1);
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
			move(active_tab->y, active_tab->x);
		}
		break;

		case 'l':
		if (((char*) get_elt(active_tab->lines, active_tab->y))[active_tab->x + 1] != '\0')
		{
			active_tab->x++;
			move(active_tab->y, active_tab->x);
		}
		break;

		case 't':
		move(height - 2, 1);
		//mode = &terminal_mode;
		break;

		case 'i':
		print_message("Insert Mode");
		mode = &insert_mode;
		break;

		case 'a':
		print_message("Insert Mode");
		active_tab->x++;
		move(active_tab->y, active_tab->x);
		mode = &insert_mode;
		break;
	}
}
