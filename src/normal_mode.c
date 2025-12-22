#include "normal_mode.h"

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
		if (active_tab->y < active_tab->lines - 1)
		{
			active_tab->y++;
			move(active_tab->y, active_tab->x);
		}
		break;

		case 'k':
		if (active_tab->y > 0)
		{
			active_tab->y--;
			move(active_tab->y, active_tab->x);
		}
		break;

		case 'l':
		if (((char*) get(active_tab->lines, active_tab->y))[active_tab->x] != '\0')
		{
			active_tab->x++;
			move(active_tab->y, active_tab->x);
		}
		break;

		case 't':
		move(height - 2, 1);
		mode = &terminal_mode;
		break;

		case 'i':
		mode = &insert_mode;
		break;

		case 'a':
		active_tab->x++;
		move(active_tab->y, active_tab->x);
		mode = &insert_mode;
		break;
	}
}
