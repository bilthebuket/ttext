#include <stdio.h>
#include <stdbool.h>
#include <ncurses.h>
#include "io_tools.h"
#include "global.h"
#include "line.h"

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

	// sorting algorithm (selection sort) is not very efficent but tabs->size is small
	Tab* tabs_sorted[tabs->size];
	for (int i = 0; i < tabs->size; i++)
	{
		tabs_sorted[i] = NULL;
	}
	for (int i = 0; i < tabs->size; i++)
	{
		Tab* max = NULL;
		for (int j = 0; j < tabs->size; j++)
		{
			Tab* t = (Tab*) get_elt(tabs, j);
			if (t == NULL)
			{
				log_error("bad get_elt() on tabs\n");
				continue;
			}

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
	if (t == NULL)
	{
		log_error("attempted to print line on NULL tab\n");
		return;
	}

	int y, x;
	getyx(stdscr, y, x);
	bool endofline;

	Line* line = (Line*) get_elt(t->lines, line_index);
	char* text;
	if (line == NULL)
	{
		log_error("found NULL line while attempting to print_line on a tab\n");
		text = NULL;
	}
	else
	{
		text = line->text;
	}
	Node* colorindex = NULL;
	if (text == NULL)
	{
		log_error("found NULL text in a line while attempting to print_line on a tab\n");
		endofline = true;
	}
	else
	{
		if (line->color_indices != NULL)
		{
			colorindex = line->color_indices->first;
			if (colorindex == NULL)
			{
				log_error("found NULL colorindex\n");
				return;
			}
		}
	}

	if (line_index < 0 || line_index >= t->lines->size)
	{
		endofline = true;
	}
	else
	{
		if (text != NULL)
		{
			int i;
			for (i = 0; text[i] != '\0' && i < t->left_column_index; i++) {}
			endofline = text[i] == '\0';
		}

		if (line->color_indices == NULL)
		{
			colorindex = NULL;
		}
		else
		{
			Node* next = NULL;

			while (colorindex->elt != NULL && ((ColorIndex*) colorindex->elt)->index < t->left_column_index)
			{
				Node* next = colorindex->next;
				if (next == NULL)
				{
					break;
				}
				else
				{
					colorindex = next;
				}
			}

			if (colorindex->elt == NULL)
			{
				log_error("found NULL elt in colorindex linked list\n");
			}

			if (next != NULL)
			{
				colorindex = colorindex->prev;
			}
		}
	}

	attron(COLOR_PAIR(WHITE_TEXT));
	for (int i = t->xpos; i <= t->xpos + t->width; i++)
	{
		if (endofline)
		{
			mvaddch(t->ypos + line_index - t->top_line_index, i, ' ');
		}
		else if (text[t->left_column_index + i - t->xpos] == '\0')
		{
			endofline = true;
			mvaddch(t->ypos + line_index - t->top_line_index, i, ' ');
		}
		else
		{
			if (colorindex != NULL)
			{
				if (colorindex->elt != NULL && ((ColorIndex*) colorindex->elt)->index == t->left_column_index + i - t->xpos)
				{
					attron(COLOR_PAIR(((ColorIndex*) colorindex->elt)->color));
					colorindex = colorindex->next;
				}
				// we have to check again because its possible we did colorindex = colorindex->next in the previous line
				if (colorindex != NULL)
				{
					if (colorindex->elt == NULL)
					{
						log_error("found NULL elt in colorindex linked list\n");
						colorindex = colorindex->next;
					}
				}
			}

			mvaddch(t->ypos + line_index - t->top_line_index, i, text[t->left_column_index + i - t->xpos]);
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

void convert_tabs_to_spaces(char* str)
{
	if (str == NULL)
	{
		log_error("found NULL in convert_tabs_to_spaces\n");
		return;
	}
	int len;
	for (len = 0; str[len] != '\0'; len++) {}

	for (int i = 0; i <= len; i++)
	{
		if (str[i] == '\t')
		{
			str[i] = ' ';
			// TAB_SIZE - 1 because we can replace the \t with a space and not have to shift anything
			for (int j = 1; j < TAB_SIZE; j++)
			{
				char c = str[i + j];
				str[i + j] = ' ';
				for (int k = i + j; k <= len; k += TAB_SIZE - 1)
				{
					char store = str[k + TAB_SIZE - 1];
					str[k + TAB_SIZE - 1] = c;
					c = store;
				}
			}
			len += TAB_SIZE - 1;
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
	if (index > 0)
	{
		Line* line = (Line*) get_elt(t->lines, index);
		Line* line_above = (Line*) get_elt(t->lines, index - 1);
		if (line == NULL || line_above == NULL)
		{
			log_error("found NULL line(s) in indent_line\n");
			return 0;
		}
		char* text = line->text;
		char* text_above = line_above->text;
		if (text == NULL || text_above == NULL)
		{
			log_error("found NULL text(s) in indent_line\n");
			return 0;
		}

		int num_spaces = 0;
		for (; text_above[num_spaces] == ' '; num_spaces++) {}

		int braces = 0;
		for (int i = num_spaces; text_above[i] != '\0'; i++)
		{
			if (text_above[i] == '{')
			{
				braces++;
			}
			if (text_above[i] == '}')
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
			int len;
			for (len = 0; text[len] != '\0'; len++) {}

			for (int j = 0; j < num_spaces; j++)
			{
				char c = text[j];
				text[j] = ' ';
				for (int k = j; k <= len; k += num_spaces)
				{
					char store = text[k + num_spaces];
					text[k + num_spaces] = c;
					c = store;
				}
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
	}
}
