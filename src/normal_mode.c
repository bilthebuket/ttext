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
	GapBuffer* gb = line->gb;
	if (gb == NULL)
	{
		log_error("found NULL gb in normal_mode\n");
		return;
	}
	switch (ch)
	{
		case 'h':
		if (active_tab->x > 0)
		{
			gb_goleft(gb);
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
			GapBuffer* gb_below = line_below->gb;
			if (gb_below == NULL)
			{
				log_error("found NULL gb in active_tab while attempting to move cursor down\n");
				return;
			}

			int i;
			if (gb_below->num_chars - 2 <= active_tab->saved_x_index)
			{
				i = gb_below->num_chars - 2;
			}
			else
			{
				i = active_tab->saved_x_index;
			}
			if (i == -1)
			{
				active_tab->x = 0;
				gb_goto(gb_below, 0);
			}
			else
			{
				active_tab->x = i;
				gb_goto(gb_below, i);
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
			Line* line_above = (Line*) get_elt(active_tab->lines, active_tab->y - 1);
			if (line_above == NULL)
			{
				log_error("found NULL line in active_tab while attempting to move cursor up\n");
				return;
			}
			GapBuffer* gb_above = line_above->gb;
			if (gb_above == NULL)
			{
				log_error("found NULL gb in active_tab while attempting to move cursor up\n");
				return;
			}

			int i;
			if (gb_above->num_chars - 2 <= active_tab->saved_x_index)
			{
				i = gb_above->num_chars - 2;
			}
			else
			{
				i = active_tab->saved_x_index;
			}
			if (i == -1)
			{
				active_tab->x = 0;
				gb_goto(gb_above, 0);
			}
			else
			{
				active_tab->x = i;
				gb_goto(gb_above, i);
			}

			active_tab->y--;

			check_top_update(active_tab);
			check_left_update(active_tab);

			move_cursor_to_tab(active_tab);
		}
		break;

		case 'l':
		if (gb_get(gb, active_tab->x + 1) != '\0')
		{
			gb_goright(gb);
			active_tab->x++;
			active_tab->saved_x_index = active_tab->x;
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
		active_tab->tab_num_flags &= ~CHANGES_SAVED;
		print_message("Insert Mode");
		mode = &insert_mode;
		break;

		case 'a':
		active_tab->tab_num_flags &= ~CHANGES_SAVED;
		print_message("Insert Mode");
		if (gb_get(gb, 0) != '\0')
		{
			gb_goright(gb);
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
		gb_goto(gb, 0);
		active_tab->x = 0;
		active_tab->saved_x_index = 0;
		check_left_update(active_tab);
		move_cursor_to_tab(active_tab);
		break;

		case '$':
		if (gb->num_chars > 1)
		{
			gb_goto(gb, gb->num_chars - 2);
			active_tab->x = gb->num_chars - 2;
			active_tab->saved_x_index = gb->num_chars - 2;
			check_right_update(active_tab);
			move_cursor_to_tab(active_tab);
		}
		break;

		case 'o':
		GapBuffer* new_gb = gb_create(NULL, -1);
		if (new_gb == NULL)
		{
			log_error("gb_create failed in normal_mode\n");
			return;
		}
		Line* new_line = malloc(sizeof(Line));
		if (new_line == NULL)
		{
			log_error("malloc failed in normal_mode\n");
			gb_free(new_gb);
			return;
		}

		new_line->gb = new_gb;
		new_line->color_indices = NULL;
		active_tab->y++;
		add(active_tab->lines, new_line, active_tab->y);
		active_tab->x = indent_line(active_tab, active_tab->y);
		print_tab(active_tab);
		move_cursor_to_tab(active_tab);
		mode = &insert_mode;
		break;

		case 'x':
		if (gb_get(gb, active_tab->x) != '\0')
		{
			active_tab->tab_num_flags &= ~CHANGES_SAVED;
			gb_rm(gb);
			update_color_indices(line);

			if (gb_get(gb, active_tab->x) == '\0' && active_tab->x != 0)
			{
				gb_goleft(gb);
				active_tab->x--;
				active_tab->saved_x_index = active_tab->x;
				check_left_update(active_tab);
				move_cursor_to_tab(active_tab);
			}

			print_tab(active_tab);
		}
		break;

		case '%':
		char c = gb_get(gb, active_tab->x);
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
			GapBuffer* gb_we_are_on = line_we_are_on->gb;
			if (gb_we_are_on == NULL)
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
				x_index = gb_we_are_on->num_chars - 2;
			}
			for (; gb_get(gb_we_are_on, x_index) != '\0' && x_index >= 0; x_index += delta)
			{
				if (gb_get(gb_we_are_on, x_index) == looking_for)
				{
					if (counter == 0)
					{
						found = true;
						gb_goto(gb_we_are_on, x_index);
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
				else if (gb_get(gb_we_are_on, x_index) == c)
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
		
		break;
	}
}
