#include <stdlib.h>
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
		if (((Line*) get_elt(active_tab->lines, active_tab->y))->text[0] != '\0')
		{
			active_tab->x++;
			if (active_tab->left_column_index + active_tab->width < active_tab->x)
			{
				active_tab->left_column_index = active_tab->x - active_tab->width;
				print_tab(active_tab);
			}
			move_cursor_to_tab(active_tab);
		}
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

		case 'o':
		char* text = malloc(sizeof(char) * LINE_SIZE);
		Line* l = malloc(sizeof(Line));
		l->text = text;
		l->color_indices = NULL;
		active_tab->y++;
		add(active_tab->lines, l, active_tab->y);
		active_tab->x = indent_line(active_tab, active_tab->y);
		print_tab(active_tab);
		move_cursor_to_tab(active_tab);
		mode = &insert_mode;
		break;

		case 'x':
		char* str = ((Line*) get_elt(active_tab->lines, active_tab->y))->text;
		if (str[active_tab->x] != '\0')
		{
			for (int i = active_tab->x; str[i] != '\0'; i++)
			{
				str[i] = str[i + 1];
			}

			update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));

			if (str[active_tab->x] == '\0' && active_tab->x != 0)
			{
				active_tab->x--;
				check_left_update(active_tab);
				move_cursor_to_tab(active_tab);
			}

			print_tab(active_tab);
		}
		break;

		case '%':
		char c = ((Line*) get_elt(active_tab->lines, active_tab->y))->text[active_tab->x];
		char looking_for;
		int delta;
		if (c == '(' || c == '[' || c == '{')
		{
			delta = 1;

			if (c == '(')
			{
				looking_for = ')';
			}
			else if (c == '[')
			{
				looking_for = ']';
			}
			else
			{
				looking_for = '}';
			}
		}
		else if (c == ')' || c == ']' || c== '}')
		{
			delta = -1;

			if (c == ')')
			{
				looking_for = '(';
			}
			else if (c == ']')
			{
				looking_for = '[';
			}
			else
			{
				looking_for = '{';
			}
		}
		else
		{
			break;
		}

		bool found = false;
		int counter = -1; // starting at negative one because the first character we see is the one the user pressed '%' on
				  // and we need to exclude it from the counter
		for (int y_index = active_tab->y; y_index >= 0 && y_index < active_tab->lines->size; y_index += delta)
		{
			char* line = ((Line*) get_elt(active_tab->lines, y_index))->text;
			int x_index;
			if (y_index == active_tab->y)
			{
				x_index = active_tab->x;
			}
			else if (delta == 1)
			{
				x_index = 0;
			}
			else
			{
				x_index = 0;
				for (; line[x_index] != '\0'; x_index++) {}
				x_index--;
			}
			for (int x_index = active_tab->x; line[x_index] != '\0' && x_index >= 0; x_index += delta)
			{
				if (line[x_index] == looking_for)
				{
					if (counter == 0)
					{
						found = true;
						active_tab->x = x_index;
						active_tab->y = y_index;

						if (delta == -1)
						{
							check_top_update(active_tab);
						}
						else
						{
							check_bottom_update(active_tab);
						}
						check_left_update(active_tab);
						check_right_update(active_tab);

						move_cursor_to_tab(active_tab);
						break;
					}
					else
					{
						counter--;
					}
				}
				else if (line[x_index] == c)
				{
					counter++;
				}
			}
			if (found)
			{
				break;
			}
		}
	}
}
