#include <stdlib.h>
#include <ncurses.h>
#include "normal_mode.h"
#include "insert_mode.h"
#include "terminal_mode.h"
#include "global.h"
#include "io_tools.h"
#include "line.h"

Tab* normal_mode(Tab* t, int ch)
{
	int line_index = pt_get_line_index(t->pt, t->y);
	if (line_index < 0)
	{
		return t;
	}

	switch (ch)
	{
		case 'h':
		if (t->x > 0)
		{
			t->x--;
			t->saved_x_index = t->x;
			check_left_update(t);
			move_cursor_to_tab(t);
		}
		break;

		case 'j':
		if (pt_get_line_index(t->pt, t->y + 1) != -1)
		{
			int line_below_index = pt_get_line_index(t->pt, t->y + 1);

			int i;
			for (i = line_below_index; pt_get(t->pt, i) != '\n' && pt_get(t->pt, i) != '\0'; i++) {}
			if (i - line_below_index - 1 <= t->saved_x_index)
			{
				i -= line_below_index + 1;
			}
			else
			{
				i = t->saved_x_index;
			}
			if (i < 0)
			{
				t->x = 0;
			}
			else
			{
				t->x = i;
			}

			t->y++;
			
			check_bottom_update(t);
			check_left_update(t);

			move_cursor_to_tab(t);
		}
		break;

		case 'k':
		if (t->y > 0)
		{
			int line_above_index = pt_get_line_index(t->pt, t->y - 1);

			int i;
			for (i = line_above_index; pt_get(t->pt, i) != '\n' && pt_get(t->pt, i) != '\0'; i++) {}
			if (i - line_above_index - 1 <= t->saved_x_index)
			{
				i -= line_above_index + 1;
			}
			else
			{
				i = t->saved_x_index;
			}
			if (i < 0)
			{
				t->x = 0;
			}
			else
			{
				t->x = i;
			}

			t->y--;

			check_top_update(t);
			check_left_update(t);

			move_cursor_to_tab(t);
		}
		break;

		case 'l':
		if (pt_get(t->pt, line_index + t->x) != '\0' && pt_get(t->pt, line_index + t->x) != '\n' && pt_get(t->pt, line_index + t->x + 1) != '\0' && pt_get(t->pt, line_index + t->x + 1) != '\n')
		{
			t->x++;
			t->saved_x_index = t->x;
			check_right_update(t);
			move_cursor_to_tab(t);
		}
		break;

		case 't':
		print_terminal();
		move_cursor_to_terminal();
		mode = &terminal_mode;
		break;

		case 'i':
		t->tab_num_flags &= ~CHANGES_SAVED;
		print_message("Insert Mode");
		mode = &insert_mode;
		break;

		case 'a':
		t->tab_num_flags &= ~CHANGES_SAVED;
		print_message("Insert Mode");
		if (pt_get(t->pt, line_index + t->x) != '\0' && pt_get(t->pt, line_index + t->x) != '\n')
		{
			t->x++;
			if (t->left_column_index + t->width < t->x)
			{
				t->left_column_index = t->x - t->width;
				print_tab(t);
			}
			move_cursor_to_tab(t);
		}
		mode = &insert_mode;
		break;

		case '0':
		t->x = 0;
		t->saved_x_index = 0;
		check_left_update(t);
		move_cursor_to_tab(t);
		break;

		case '$':
		{
		int len;
		for (len = 0; pt_get(t->pt, line_index + len) != '\n' && pt_get(t->pt, line_index + len) != '\0'; len++) {}
		t->x = len - 1;
		t->saved_x_index = len - 1;
		check_right_update(t);
		move_cursor_to_tab(t);
		break;
		}

		case 'o':
		{
		int len;
		for (len = 0; pt_get(t->pt, line_index + len) != '\n' && pt_get(t->pt, line_index + len) != '\0'; len++) {}
		pt_insert(t->pt, '\n', line_index + len);

		t->y++;
		t->x = indent_line(t, t->y);
		print_tab(t);
		move_cursor_to_tab(t);
		mode = &insert_mode;
		break;
		}

		case 'x':
		if (pt_get(t->pt, line_index + t->x) != '\n' && pt_get(t->pt, line_index + t->x) != '\0')
		{
			t->tab_num_flags &= ~CHANGES_SAVED;
			pt_rm(t->pt, line_index + t->x);

			if ((pt_get(t->pt, line_index + t->x) == '\0' || pt_get(t->pt, line_index + t->x) == '\n') && t->x > 0)
			{
				t->x--;
				t->saved_x_index = t->x;
				check_left_update(t);
				move_cursor_to_tab(t);
			}

			print_line(t, t->y);
		}
		break;

		case '%':
		char c = pt_get(t->pt, line_index + t->x);
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
		for (int y_index = t->y; y_index >= 0 && pt_get_line_index(t->pt, y_index) != -1; y_index += delta)
		{
			int line_we_are_on = pt_get_line_index(t->pt, y_index);
			int len;
			for (len = 0; pt_get(t->pt, line_we_are_on + len) != '\0' && pt_get(t->pt, line_we_are_on + len) != '\n'; len++) {}

			int x_index;
			if (y_index == t->y)
			{
				x_index = t->x;
			}
			else if (delta == 1)
			{
				x_index = 0;
			}
			else
			{
				x_index = len - 1;
			}
			for (; pt_get(t->pt, line_we_are_on + x_index) != '\0' && x_index >= 0 && x_index < len; x_index += delta)
			{
				if (pt_get(t->pt, line_we_are_on + x_index) == looking_for)
				{
					if (counter == 0)
					{
						found = true;
						t->x = x_index;
						t->y = y_index;

						if (delta == -1)
						{
							check_top_update(t);
						}
						else
						{
							check_bottom_update(t);
						}
						check_left_update(t);
						check_right_update(t);

						move_cursor_to_tab(t);
						break;
					}
					else
					{
						counter--;
					}
				}
				else if (pt_get(t->pt, line_we_are_on + x_index) == c)
				{
					counter++;
				}
			}
			if (found)
			{
				break;
			}
		}

		t->saved_x_index = t->x;
		
		break;
	}

	return t;
}
