#include <stdlib.h>
#include <ncurses.h>
#include "LL.h"
#include "insert_mode.h"
#include "normal_mode.h"
#include "tab.h"
#include "global.h"
#include "io_tools.h"
#include "line.h"

void insert_mode(int ch)
{
	Line* line = (Line*) get_elt(active_tab->lines, active_tab->y);
	if (line == NULL)
	{
		log_error("Accessing line in tab results in NULL\n");
		return;
	}
	char* text = line->text;
	if (text == NULL)
	{
		log_error("Accessing text in line results in NULL\n");
		return;
	}

	int i;

	switch (ch)
	{
		default:
		for (i = active_tab->x; text[i] != '\0'; i++) {}
		if (i != LINE_SIZE - 1)
		{
			for (; i >= active_tab->x; i--)
			{
				text[i + 1] = text[i];
			}
			text[active_tab->x] = ch;

			if (ch == '\t')
			{
				convert_tabs_to_spaces(text);
				active_tab->x += TAB_SIZE - 1;
			}
			if (ch == '}')
			{
				bool indent = true;
				int j;
				for (j = 0; text[j] != '}'; j++)
				{
					if (text[j] != ' ')
					{
						indent = false;
						break;
					}
				}
				if (indent)
				{
					int amount;
					if (j >= TAB_SIZE)
					{
						amount = TAB_SIZE;
					}
					else
					{
						amount = j;
					}

					for (; text[j] != '\0'; j++)
					{
						text[j - amount] = text[j];
					}
					text[j - amount] = '\0';
					active_tab->x -= amount;
				}
			}

			active_tab->x++;
			check_right_update(active_tab);
			move_cursor_to_tab(active_tab);

			update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));
			print_line(active_tab, active_tab->y);
		}
		break;

		case BACKSPACE_KEYCODE2:
		if (active_tab->x > 0)
		{
			for (int i = active_tab->x - 1; text[i] != '\0'; i++)
			{
				text[i] = text[i + 1];
			}

			active_tab->x--;
			check_left_update(active_tab);
			move_cursor_to_tab(active_tab);

			update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));
			print_line(active_tab, active_tab->y);
		}
		else if (active_tab->y > 0)
		{
			Line* line_above = (Line*) get_elt(active_tab->lines, active_tab->y - 1);
			if (line_above == NULL)
			{
				log_error("Accessing line in tab results in NULL\n");
				return;
			}
			char* text_above = line_above->text;
			if (text_above == NULL)
			{
				log_error("Accessing text in line results in NULL\n");
				return;
			}

			for (i = 0; text_above[i] != '\0'; i++) {}
			int len = i;
			for (; text[i - len] != '\0' && i < LINE_SIZE; i++)
			{
				text_above[i] = text[i - len];
			}
			if (text[i - len] != '\0')
			{
				text[len] = '\0';
				print_message("Operation would cause a line to exceed the maximum line size");
			}
			else
			{
				rm(active_tab->lines, active_tab->y);
				text_above[i] = '\0';
				free(text);

				active_tab->x = i;
				active_tab->y--;

				check_left_update(active_tab);
				check_right_update(active_tab);
				check_top_update(active_tab);
				move_cursor_to_tab(active_tab);

				update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));
				print_tab(active_tab);
			}
		}
		break;

		case ESCAPE_KEYCODE:
		print_message("Normal Mode");
		active_tab->saved_x_index = active_tab->x;
		mode = &normal_mode;
		break;

		case ENTER_KEYCODE1:
		char* buf = malloc(sizeof(char) * LINE_SIZE);
		if (buf == NULL)
		{
			log_error("malloc failure\n");
			return;
		}

		for (i = active_tab->x; text[i] != '\0'; i++)
		{
			buf[i - active_tab->x] = text[i];
		}
		text[active_tab->x] = '\0';
		buf[i - active_tab->x] = '\0';

		Line* l = malloc(sizeof(Line));
		if (l == NULL)
		{
			log_error("malloc failure\n");
			free(buf);
			return;
		}

		l->text = buf;
		l->color_indices = NULL;
		add(active_tab->lines, l, active_tab->y + 1);

		active_tab->y++;
		active_tab->x = indent_line(active_tab, active_tab->y);

		check_left_update(active_tab);
		check_bottom_update(active_tab);
		move_cursor_to_tab(active_tab);

 		update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y));
 		update_color_indices((Line*) get_elt(active_tab->lines, active_tab->y - 1));
		print_tab(active_tab);
		break;
	}
}
