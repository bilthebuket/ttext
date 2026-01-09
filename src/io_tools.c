#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include "io_tools.h"
#include "global.h"
#include "line.h"

void print_tab(Tab* t)
{
	int y, x;
	getyx(stdscr, y, x);
	for (int i = t->top_line_index; i <= t->top_line_index + t->height; i++)
	{
		print_line(t, i);
	}
	move(y, x);
}

void print_screen(void)
{
	int y, x;
	getyx(stdscr, y, x);

	// using selection sort becuase tabs->size is small and i'm lazy
	Tab* tabs_sorted[tabs->size];
	for (int i = 0; i < tabs->size; i++)
	{
		Tab* max = NULL;
		for (int j = 0; j < tabs->size; j++)
		{
			Tab* t = (Tab*) get_elt(tabs, j);
			bool already_sorted = false;
			for (int k = 0; k < i; k++)
			{
				if (t == tabs_sorted[k])
				{
					already_sorted = true;
					break;
				}
			}
			if (already_sorted)
			{
				continue;
			}
			if (max == NULL)
			{
				max = t;
			}
			else if ((t->z_index_changes_saved & ~CHANGES_SAVED) > (max->z_index_changes_saved & ~CHANGES_SAVED))
			{
				max = t;
			}
		}
		tabs_sorted[i] = max;
	}

	for (int i = 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			mvaddch(j, i, ' ');
		}
	}
	for (int i = 0; i < tabs->size; i++)
	{
		print_tab(tabs_sorted[i]);
	}
	print_tab(terminal);

	move(y,x);
}

void print_line(Tab* t, int line_index)
{
	int y, x;
	getyx(stdscr, y, x);
	bool endofline;
	char* line;
	Node* colorindex;
	if (line_index < 0 || line_index >= t->lines->size)
	{
		endofline = true;
	}
	else
	{
		line = ((Line*) get_elt(t->lines, line_index))->text;
		int i;
		for (i = 0; line[i] != '\0' && i < t->left_column_index; i++) {}
		endofline = line[i] == '\0';

		Line* l = (Line*) get_elt(t->lines, line_index);
		if (l->color_indices == NULL)
		{
			colorindex = NULL;
		}
		else
		{
			colorindex = ((Line*) get_elt(t->lines, line_index))->color_indices->first;
		}
	}

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
			if (colorindex != NULL)
			{
				if (((ColorIndex*) colorindex->elt)->index == i)
				{
					attron(COLOR_PAIR(((ColorIndex*) colorindex->elt)->color));
					colorindex = colorindex->next;
				}
			}
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
