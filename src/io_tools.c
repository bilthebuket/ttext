#include <stdio.h>
#include <ncurses.h>
#include "io_tools.h"
#include "global.h"

void print_tab(Tab* t)
{
	for (int i = 0; i < t->lines->size; i++)
	{
		for (int j = 0; j < width; j++)
		{
			mvaddch(i, j, ' ');
		}
		mvaddstr(i, 0, (char*) get_elt(t->lines, i));
	}
}

void print_message(char* str)
{

}

void clear_message_line(void)
{

}
