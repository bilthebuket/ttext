#include <stdio.h>
#include <ncurses.h>
#include "io_tools.h"
#include "global.h"

void print_tab(Tab* t)
{
	int y, x;
	getyx(stdscr, y, x);
	for (int i = 0; i < t->lines->size; i++)
	{
		for (int j = 0; j < width; j++)
		{
			mvaddch(i, j, ' ');
		}
		mvaddstr(i, 0, (char*) get_elt(t->lines, i));
	}
	move(y, x);
}

void print_line(Tab* t, int line)
{
	int y, x;
	getyx(stdscr, y, x);
	for (int i = 0; i < width; i++)
	{
		mvaddch(line, i, ' ');
	}
	mvaddstr(line, 0, (char*) get_elt(t->lines, line));
	move(y, x);
}

void print_message(char* str)
{
	int y, x;
	getyx(stdscr, y, x);
	for (int i = 0; i < width; i++)
	{
		mvaddch(height - 1, i, ' ');
	}
	mvaddstr(height - 1, 0, str);
	move(y, x);
}

void clear_message_line(void)
{
	int y, x;
	getyx(stdscr, y, x);
	for (int i = 0; i < width; i++)
	{
		mvaddch(height - 1, i, ' ');
	}
	move(y, x);
}
