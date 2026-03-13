#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include "io_tools.h"
#include "global.h"
#include "line.h"
#include "terminal_mode.h"

void print_tab(Tab* t)
{
	if (t == NULL)
	{
		return;
	}

	int y, x;
	getyx(stdscr, y, x);
	for (int i = t->top_line_index; i <= t->top_line_index + t->height && i - t->top_line_index < height - 1; i++)
	{
		print_line(t, i);
	}
	move(y, x);
}

void print_screen(void)
{
	int y, x;
	getyx(stdscr, y, x);

	if (tabs == NULL)
	{
		return;
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
		print_tab((Tab*) ll_get_elt(tabs, i));
	}
	print_terminal();

	move(y,x);
}

void print_line(Tab* t, int line_index)
{
	if (t == NULL)
	{
		log_error("attempted to print line on NULL tab\n");
		return;
	}

	int y, x;
	getyx(stdscr, y, x);
	bool endofline = false;

	// iterator is created inside print_line and not print_tab because there are pros and cons to both and this is easier
	// implementing the iterator at the print_tab level would mean we have to iterate over all characters that are off screen to get to the next line
	// implementing the iterator at the print_line level means we have to traverse the tree for every line
	// the performance difference is O(n) vs O(mlogb), where n = # of chars off screen, m = # of lines on screen, and b = # of nodes in tree
	// this way is better for having several smalls tabs open
	PieceIterator pi;
	if (!pt_iterator_init(t->pt, &pi, pt_get_line_index(t->pt, line_index)))
	{
		endofline = true;
	}
	else
	{
		for (int i = 0; i < t->left_column_index; i++)
		{
			char c = pt_iterate(&pi);
			if (c == '\n' || c == '\0')
			{
				endofline = true;
				break;
			}
		}
	}

	if (t->pt != NULL)
	{
		int char_index = pt_get_line_index(t->pt, line_index);
		if (!endofline && char_index < 0)
		{
			return;
		}
		for (int i = t->xpos; i <= t->xpos + t->width; i++)
		{
			if (endofline)
			{
				mvaddch(t->ypos + line_index - t->top_line_index, i, ' ');
				continue;
			}

			char c = pt_iterate(&pi);
			if (c == '\n' || c == '\0')
			{
				endofline = true;
				mvaddch(t->ypos + line_index - t->top_line_index, i, ' ');
			}
			else
			{
				//TODO: make iterator for colors like the one for pieces
				attron(COLOR_PAIR(pt_get_color(t->pt, char_index + t->left_column_index + i - t->xpos)));
				mvaddch(t->ypos + line_index - t->top_line_index, i, c);
			}
		}
	}
	else
	{
		Line* line = (Line*) ll_get_elt(t->lines, line_index);
		GapBuffer* gb;
		if (line == NULL)
		{
			gb = NULL;
		}
		else
		{
			gb = line->gb;
		}
		if (gb == NULL)
		{
			endofline = true;
		}

		if (line_index < 0 || line_index >= t->lines->size)
		{
			endofline = true;
		}
		else
		{
			if (gb != NULL)
			{
				int i;
				for (i = 0; gb_get(gb, i) != '\0' && i < t->left_column_index; i++) {}
				endofline = gb_get(gb, i) == '\0';
			}
		}

		attron(COLOR_PAIR(WHITE_TEXT));
		for (int i = t->xpos; i <= t->xpos + t->width; i++)
		{
			if (endofline)
			{
				mvaddch(t->ypos + line_index - t->top_line_index, i, ' ');
			}
			else if (gb_get(gb, t->left_column_index + i - t->xpos) == '\0')
			{
				endofline = true;
				mvaddch(t->ypos + line_index - t->top_line_index, i, ' ');
			}
			else
			{
				mvaddch(t->ypos + line_index - t->top_line_index, i, gb_get(gb, t->left_column_index + i - t->xpos));
			}
		}
	}
	move(y, x);
}

void print_message(const char* const str)
{
	if (str == NULL)
	{
		log_error("found NULL string in print_message\n");
		return;
	}
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
	if (t == NULL)
	{
		log_error("found NULL tab in move_cursor_to_tab\n");
		return;
	}
	move(t->ypos + t->y - t->top_line_index, t->xpos + t->x - t->left_column_index);
}

void check_left_update(Tab* t)
{
	if (t == NULL)
	{
		log_error("found NULL in check_left_update\n");
		return;
	}
	if (t->left_column_index > t->x)
	{
		t->left_column_index = t->x;
		print_tab(t);
	}
}

void check_right_update(Tab* t)
{
	if (t == NULL)
	{
		log_error("found NULL in check_right_update\n");
		return;
	}
	if (t->left_column_index + t->width < t->x)
	{
		t->left_column_index = t->x - t->width;
		print_tab(t);
	}
}

void check_top_update(Tab* t)
{
	if (t == NULL)
	{
		log_error("found NULL in check_top_update\n");
		return;
	}
	if (t->y < t->top_line_index)
	{
		t->top_line_index = t->y;
		print_tab(t);
	}
}

void check_bottom_update(Tab* t)
{
	if (t == NULL)
	{
		log_error("found NULL in check_bottom_update\n");
		return;
	}
	if (t->y > t->top_line_index + t->height)
	{
		t->top_line_index = t->y - t->height;
		print_tab(t);
	}
}

void convert_tabs_to_spaces(GapBuffer* gb)
{
	if (gb == NULL)
	{
		log_error("found NULL in convert_tabs_to_spaces\n");
		return;
	}

	for (int i = 0; i < gb->num_chars - 1; i++)
	{
		if (gb_get(gb, i) == '\t')
		{
			gb_goto(gb, i);
			gb_rm(gb);
			for (int j = 0; j < TAB_SIZE; j++)
			{
				gb_insert(gb, ' ');
			}
		}
	}
}

int indent_line(Tab* t, int index)
{
	if (t == NULL)
	{
		log_error("found NULL tab in indent_line\n");
		return 0;
	}
	if (index >= 0)
	{
		int line_index = pt_get_line_index(t->pt, index);
		int line_above_index = pt_get_line_index(t->pt, index - 1);
		if (line_index < 0 || line_above_index < 0)
		{
			log_error("could not get line indicies in indent_line\n");
			return 0;
		}

		int num_spaces = 0;
		for (; pt_get(t->pt, line_above_index + num_spaces) == ' '; num_spaces++) {}

		int braces = 0;
		for (int i = num_spaces; pt_get(t->pt, line_above_index + i) != '\n' && pt_get(t->pt, line_above_index + i) != '\0'; i++)
		{
			if (pt_get(t->pt, line_above_index + i) == '{')
			{
				braces++;
			}
			if (pt_get(t->pt, line_above_index + i) == '}')
			{
				braces--;
			}
		}
		if (braces > 0)
		{
			num_spaces += TAB_SIZE * braces;
		}

		if (num_spaces > 0)
		{
			for (int j = 0; j < num_spaces; j++)
			{
				pt_insert(t->pt, ' ', line_index + j);
			}

			return num_spaces;
		}
	}

	return 0;
}

void log_error(const char* str)
{
	if (error_log != NULL)
	{
		fprintf(error_log, str);
		fflush(error_log);
	}
}
