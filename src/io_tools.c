#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include "io_tools.h"
#include "global.h"

void print_tab(Tab* t)
{
	int y, x;
	getyx(stdscr, y, x);
	for (int i = t->ypos; i <= t->ypos + t->height; i++)
	{
		bool endofline = true;
		char* line;
		if (t->top_line_index + i - t->ypos < t->lines->size)
		{
			line = (char*) get_elt(t->lines, t->top_line_index + i - t->ypos);
			int j;
			for (j = 0; line[j] != '\0' && j < t->left_column_index; j++) {}
			endofline = line[j] == '\0';
		}

		for (int j = t->xpos; j <= t->xpos + t->width; j++)
		{
			if (endofline)
			{
				mvaddch(i, j, ' ');
			}
			else if (line[t->left_column_index + j - t->xpos] == '\0')
			{
				endofline = true;
				mvaddch(i, j, ' ');
			}
			else
			{
				mvaddch(i, j, line[t->left_column_index + j - t->xpos]);
			}
		}
	}
	move(y, x);
}

void print_line(Tab* t, int line_index)
{
	int y, x;
	getyx(stdscr, y, x);
	char* line = (char*) get_elt(t->lines, line_index);
	int i;
	for (i = 0; line[i] != '\0' && i < t->left_column_index; i++) {}
	bool endofline = line[i] == '\0';

	for (int i = t->xpos; i <= t->xpos + t->width; i++)
	{
		if (endofline)
		{
			mvaddch(t->ypos + line_index - t->top_line_index, i, ' ');
		}
		else if (line[t->left_column_index + i - t->xpos] == '\0')
		{
			endofline = true;
			mvaddch(t->ypos + line_index - t->top_line_index, i, ' ');
		}
		else
		{
			mvaddch(t->ypos + line_index - t->top_line_index, i, line[t->left_column_index + i - t->xpos]);
		}
	}
	move(y, x);
}

void print_message(const char* const str)
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

void move_cursor_to_tab(Tab* t)
{
	move(t->ypos + t->y - t->top_line_index, t->xpos + t->x - t->left_column_index);
}

void check_left_update(Tab* t)
{
	if (t->left_column_index > t->x)
	{
		t->left_column_index = t->x;
		print_tab(t);
	}
}

void check_right_update(Tab* t)
{
	if (t->left_column_index + t->width < t->x)
	{
		t->left_column_index = t->x - t->width;
		print_tab(t);
	}
}

void check_top_update(Tab* t)
{
	if (t->y < t->top_line_index)
	{
		t->top_line_index = t->y;
		print_tab(t);
	}
}

void check_bottom_update(Tab* t)
{
	if (t->y > t->top_line_index + t->height)
	{
		t->top_line_index = t->y - t->height;
		print_tab(t);
	}
}
