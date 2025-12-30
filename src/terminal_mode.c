#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include "terminal_mode.h"
#include "normal_mode.h"
#include "global.h"
#include "io_tools.h"

static void make_input_line(void)
{
	char* line = malloc(sizeof(char) * LINE_SIZE);
	line[0] = '\0';
	add(terminal->lines, line, terminal->lines->size);
	terminal->x = 0;
	terminal->y++;
	check_bottom_update(terminal);
	move_cursor_to_tab(terminal);
}

void terminal_mode(int ch)
{
	char* line = (char*) get_elt(terminal->lines, terminal->y);
	int i;

	switch (ch)
	{
		default:
		for (i = terminal->x; line[i] != '\0'; i++) {}
		if (i != LINE_SIZE - 1)
		{
			for (; i >= terminal->x; i--)
			{
				line[i + 1] = line[i];
			}
			line[terminal->x] = ch;

			terminal->x++;
			check_right_update(terminal);
			move_cursor_to_tab(terminal);

			print_line(terminal, terminal->y);
		}
		break;

		case ENTER_KEYCODE1:
		if (line[0] == ':')
		{
			char* ptr = &line[1];
			bool only_one_arg = false;

			int i = 0;
			for (; ptr[i] != ' ' && ptr[i] != '\0'; i++) {}

			if (ptr[i] == '\0')
			{
				only_one_arg = true;
			}
			ptr[i] = '\0';

			if (!strcmp(ptr, "tabnew"))
			{
				if (only_one_arg)
				{
					print_message("Please pass filename as argument");
					make_input_line();
					return;
				}

				ptr[i] = ' ';
				ptr = &ptr[i + 1];
				active_tab = make_tab(ptr);
				add(tabs, active_tab, tabs->size);
				active_tab_index = tabs->size - 1;
				print_tab(active_tab);
			}
			else if (!strcmp(ptr, "tabn"))
			{
				if (active_tab_index == tabs->size - 1)
				{
					active_tab_index = 0;
				}
				else
				{
					active_tab_index++;
				}

				active_tab = (Tab*) get_elt(tabs, active_tab_index);
				print_tab(active_tab);
			}
			else if (!strcmp(ptr, "tabp"))
			{
				if (active_tab_index == 0)
				{
					active_tab_index = tabs->size - 1;
				}
				else
				{
					active_tab_index--;
				}

				active_tab = (Tab*) get_elt(tabs, active_tab_index);
				print_tab(active_tab);
			}
			else if (!strcmp(ptr, "tab"))
			{
				if (only_one_arg)
				{
					print_message("Please pass the index of the tab to switch to");
					make_input_line();
					return;
				}

				ptr[i] = ' ';
				ptr = &ptr[i + 1];
				int index = atoi(ptr);

				if (index < 0 || index >= tabs->size)
				{
					print_message("Index invalid");
					make_input_line();
					return;
				}

				active_tab = (Tab*) get_elt(tabs, index);
				print_tab(active_tab);
			}
			else if (!strcmp(ptr, "rs"))
			{
				int amount; 

				int* num_to_change1 = NULL;
				int sign1;

				int* num_to_change2 = NULL;
				int sign2;

				if (only_one_arg)
				{
					print_message("Usage: :rs <top/bottom/left/right> <add/sub> <amount>");
					make_input_line();
					return;
				}

				ptr[i] = ' ';
				ptr = &ptr[i + 1];
				for (i = 0; ptr[i] != '\0' && ptr[i] != ' '; i++) {}

				if (ptr[i] == '\0')
				{
					print_message("Usage: :rs <top/bottom/left/right> <add/sub> <amount>");
					make_input_line();
					return;
				}

				ptr[i] = '\0';

				if (!strcmp(ptr, "top"))
				{
					num_to_change1 = &active_tab->ypos;
					num_to_change2 = &active_tab->height;
				}
				else if (!strcmp(ptr, "bottom"))
				{
					num_to_change1 = &active_tab->height;
				}
				else if (!strcmp(ptr, "left"))
				{
					num_to_change1 = &active_tab->xpos;
					num_to_change2 = &active_tab->width;
				}
				else if (!strcmp(ptr, "right"))
				{
					num_to_change1 = &active_tab->width;
				}

				ptr[i] = ' ';
				ptr = &ptr[i + 1];
				for (i = 0; ptr[i] != '\0' && ptr[i] != ' '; i++) {}

				if (ptr[i] == '\0')
				{
					print_message("Usage: :rs <top/bottom/left/right> <add/sub> <amount>");
					make_input_line();
					return;
				}

				ptr[i] = '\0';

				if (!strcmp(ptr, "add"))
				{
					if (num_to_change2 == NULL)
					{
						sign1 = 1;
					}
					else
					{
						sign1 = -1;
						sign2 = 1;
					}
				}
				else if (!strcmp(ptr, "sub"))
				{
					if (num_to_change2 == NULL)
					{
						sign1 = -1;
					}
					else
					{
						sign1 = 1;
						sign2 = -1;
					}
				}

				ptr[i] = ' ';
				ptr = &ptr[i + 1];
				amount = atoi(ptr);
				amount = atoi(ptr);

				if (num_to_change1 != NULL)
				{
					*num_to_change1 = *num_to_change1 + sign1 * amount;
				}
				if (num_to_change2 != NULL)
				{
					*num_to_change2 = *num_to_change2 + sign2 * amount;
				}

				print_tab(active_tab);
				move_cursor_to_tab(active_tab);
			}
			else if (!strcmp(ptr, "mv"))
			{
				if (only_one_arg)
				{
					print_message("Ussage: :mv <left/right/up/down> <amount>");
					make_input_line();
					return;
				}

				int* num_to_change = NULL;
				int sign;
				int amount;

				ptr[i] = ' ';
				ptr = &ptr[i + 1];
				for (i = 0; ptr[i] != ' ' && ptr[i] != '\0'; i++) {}
				if (ptr[i] == '\0')
				{
					print_message("Ussage: :mv <left/right/up/down> <amount>");
					make_input_line();
					return;
				}
				ptr[i] = '\0';

				if (!strcmp(ptr, "left"))
				{
					num_to_change = &active_tab->xpos;
					sign = -1;
				}
				else if (!strcmp(ptr, "right"))
				{
					num_to_change = &active_tab->xpos;
					sign = 1;
				}
				else if (!strcmp(ptr, "up"))
				{
					num_to_change = &active_tab->ypos;
					sign = -1;
				}
				else if (!strcmp(ptr, "down"))
				{
					num_to_change = &active_tab->ypos;
					sign = 1;
				}

				ptr[i] = ' ';
				ptr = &ptr[i + 1];
				amount = atoi(ptr);

				if (num_to_change != NULL)
				{
					*num_to_change = *num_to_change + amount * sign;
				}

				print_tab(active_tab);
				move_cursor_to_tab(active_tab);
				make_input_line();
			}
			else if (!strcmp(ptr, "q"))
			{
				if (active_tab->changes_saved)
				{
					if (active_tab_index == tabs->size - 1)
					{
						if (tabs->size == 1)
						{
							terminate = true;
						}
						else
						{
							active_tab = (Tab*) get_elt(tabs, active_tab_index - 1);
							free_tab((Tab*) rm(tabs, active_tab_index));
							active_tab_index--;
						}
					}
					else
					{
						active_tab = (Tab*) get_elt(tabs, active_tab_index + 1);
						free_tab((Tab*) rm(tabs, active_tab_index));
					}
				}
			}
			else if (!strcmp(ptr, "q!"))
			{
				if (active_tab_index == tabs->size - 1)
				{
					if (tabs->size == 1)
					{
						terminate = true;
					}

					active_tab = (Tab*) get_elt(tabs, active_tab_index - 1);
					free_tab((Tab*) rm(tabs, active_tab_index));
					active_tab_index--;
				}
				else
				{
					active_tab = (Tab*) get_elt(tabs, active_tab_index + 1);
					free_tab((Tab*) rm(tabs, active_tab_index));
				}
			}
			else if (!strcmp(ptr, "w"))
			{
				if (active_tab->fname == NULL)
				{
					char* fname = malloc(sizeof(char) * 10);
					fname[0] = 'u';
					fname[1] = 'n';
					fname[2] = 't';
					fname[3] = 'i';
					fname[4] = 't';
					fname[5] = '.';
					fname[6] = 't';
					fname[7] = 'x';
					fname[8] = 't';
					active_tab->fname = fname;
				}

				FILE* f = fopen(active_tab->fname, "w");
				for (int i = 0; i < active_tab->lines->size; i++)
				{
					fprintf(f, "%s\n", (char*) get_elt(active_tab->lines, i));
				}
				fclose(f);
				active_tab->changes_saved = true;
			}
			else if (!strcmp(ptr, "findreplace"))
			{

			}

			make_input_line();
			print_tab(terminal);
		}
		else
		{
			int i = 0;
			for (; line[i] != '\0'; i++) {}
			line[i] = '\n';
			line[i + 1] = '\0';
			write(master_fd, line, i + 2);
			line[0] = '\0';
			terminal->x = 0;
			check_bottom_update(terminal);
			move_cursor_to_tab(terminal);
		}
		break;

		case BACKSPACE_KEYCODE2:
		if (terminal->x > 0)
		{
			for (int i = terminal->x - 1; line[i] != '\0'; i++)
			{
				line[i] = line[i + 1];
			}

			terminal->x--;
			check_left_update(terminal);
			move_cursor_to_tab(terminal);

			print_line(terminal, terminal->y);
		}
		break;

		case ESCAPE_KEYCODE:
		print_tab(active_tab);
		move_cursor_to_tab(active_tab);
		mode = &normal_mode;
		break;
	}
}

void* listener_func(void*)
{
	while (!terminate)
	{
		listener_buf = malloc(sizeof(char) * LINE_SIZE);
		int bytes_read = read(master_fd, listener_buf, LINE_SIZE);
		if (bytes_read > 0)
		{
			listener_buf[bytes_read] = '\0';
			sem_wait(&sem);
			add(terminal->lines, listener_buf, terminal->lines->size - 1);
			terminal->y++;
			check_bottom_update(terminal);
			if (mode == &terminal_mode)
			{
				move_cursor_to_tab(terminal);
			}
			print_tab(terminal);
			refresh();
			sem_post(&sem);
		}
		else
		{
			sem_wait(&sem);
			free(listener_buf);
			listener_buf = NULL;
			sem_post(&sem);
		}
	}

	return NULL;
}
