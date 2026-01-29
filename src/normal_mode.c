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
	Line* line = (Line*) get_elt(active_tab->lines, active_tab->y);
	if (line == NULL)
	{
		log_error("reached NULL line in normal_mode\n");
		return;
	}
	char* text = line->text;
	switch (ch)
	{
		case 'h':
		if (active_tab->x > 0)
		{
			active_tab->x--;
			active_tab->saved_x_index = active_tab->x;
			check_left_update(active_tab);
			move_cursor_to_tab(active_tab);
		}
		break;

		case 'j':
		if (active_tab->y < active_tab->lines->size - 1)
		{
			Line* line_below = (Line*) get_elt(active_tab->lines, active_tab->y + 1);
			if (line_below == NULL)
			{
				log_error("found NULL line in active_tab while attempting to move cursor down\n");
				return;
			}
			char* text_below = line_below->text;
			if (text_below != NULL)
			{
				int i;
				for (i = 0; text_below[i] != '\0' && i <= active_tab->saved_x_index; i++) {}
				if (text_below[i] == '\0')
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
				else
				{
					active_tab->x = active_tab->saved_x_index;
				}

				active_tab->y++;
				
				check_bottom_update(active_tab);
				check_left_update(active_tab);

				move_cursor_to_tab(active_tab);
			}
		}
		break;

		case 'k':
		if (active_tab->y > 0)
		{
			Line* line_above = (Line*) get_elt(active_tab->lines, active_tab->y - 1);
			if (line_above == NULL)
			{
				log_error("found NULL line in active_tab while attempting to move cursor up\n");
				return;
			}
			char* text_above = line_above->text;
			if (text_above != NULL)
			{
				int i;
				for (i = 0; text_above[i] != '\0' && i <= active_tab->saved_x_index; i++) {}
				if (text_above[i] == '\0')
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
				else
				{
					active_tab->x = active_tab->saved_x_index;
				}

				active_tab->y--;

				check_top_update(active_tab);
				check_left_update(active_tab);

				move_cursor_to_tab(active_tab);
			}
		}
		break;

		case 'l':
		if (text != NULL)
		{
			if (text[active_tab->x + 1] != '\0')
			{
				active_tab->x++;
				active_tab->saved_x_index = active_tab->x;
				check_right_update(active_tab);
				move_cursor_to_tab(active_tab);
			}
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
		if (text[0] != '\0')
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
		active_tab->saved_x_index = 0;
		check_left_update(active_tab);
		move_cursor_to_tab(active_tab);
		break;

		case '$':
		if (text != NULL)
		{
			int len = 0;
			for (; text[len] != '\0'; len++) {}
			if (len > 0)
			{
				active_tab->x = len - 1;
				active_tab->saved_x_index = len - 1;
				check_right_update(active_tab);
				move_cursor_to_tab(active_tab);
			}
		}
		break;

		case 'o':
		char* new_text = malloc(sizeof(char) * LINE_SIZE);
		if (new_text == NULL)
		{
			log_error("malloc failed in normal_mode\n");
			return;
		}
		Line* new_line = malloc(sizeof(Line));
		if (new_line == NULL)
		{
			log_error("malloc failed in normal_mode\n");
			free(new_text);
			return;
		}

		new_line->text = new_text;
		new_line->color_indices = NULL;
		active_tab->y++;
		add(active_tab->lines, new_line, active_tab->y);
		active_tab->x = indent_line(active_tab, active_tab->y);
		print_tab(active_tab);
		move_cursor_to_tab(active_tab);
		mode = &insert_mode;
		break;

		case 'x':
		if (text != NULL)
		{
			if (text[active_tab->x] != '\0')
			{
				for (int i = active_tab->x; text[i] != '\0'; i++)
				{
					text[i] = text[i + 1];
				}

				update_color_indices(line);

				if (text[active_tab->x] == '\0' && active_tab->x != 0)
				{
					active_tab->x--;
					active_tab->saved_x_index = active_tab->x;
					check_left_update(active_tab);
					move_cursor_to_tab(active_tab);
				}

				print_tab(active_tab);
			}
		}
		break;

		case '%':
		if (text != NULL)
		{
			char c = text[active_tab->x];
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
				Line* line_we_are_on = (Line*) get_elt(active_tab->lines, y_index);
				if (line_we_are_on == NULL)
				{
					log_error("found NULL line in normal_mode\n");
					return;
				}
				char* text_we_are_on = line_we_are_on->text;
				if (text_we_are_on == NULL)
				{
					continue;
				}

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
					for (; text_we_are_on[x_index] != '\0'; x_index++) {}
					x_index--;
				}
				for (int x_index = active_tab->x; text_we_are_on[x_index] != '\0' && x_index >= 0; x_index += delta)
				{
					if (text_we_are_on[x_index] == looking_for)
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
					else if (text_we_are_on[x_index] == c)
					{
						counter++;
					}
				}
				if (found)
				{
					break;
				}
			}

			active_tab->saved_x_index = active_tab->x;
		}
		
		break;
	}
}
