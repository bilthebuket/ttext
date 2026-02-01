#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include "terminal_mode.h"
#include "normal_mode.h"
#include "global.h"
#include "io_tools.h"
#include "line.h"

static void make_input_line(void)
{
	char* line = malloc(sizeof(char) * LINE_SIZE);
	if (line == NULL)
	{
		log_error("malloc failed in make_input_line\n");
		return;
	}
	line[0] = '\0';

	Line* l = malloc(sizeof(Line));
	if (l == NULL)
	{
		log_error("malloc failed in make_input_line\n");
		return;
	}
	l->text = line;
	l->color_indices = NULL;
	add(terminal->lines, l, terminal->lines->size);
	terminal->x = 0;
	terminal->y++;
	check_bottom_update(terminal);
	move_cursor_to_tab(terminal);
}

void terminal_mode(int ch)
{
	Line* line = (Line*) get_elt(terminal->lines, terminal->y);
	if (line == NULL)
	{
		log_error("NULL line in terminal_mode\n");
		return;
	}
	char* text = line->text;
	if (text == NULL)
	{
		log_error("NULL text in terminal_mode\n");
		return;
	}
	int i;

	switch (ch)
	{
		default:
		for (i = terminal->x; text[i] != '\0'; i++) {}
		if (i != LINE_SIZE - 1)
		{
			for (; i >= terminal->x; i--)
			{
				text[i + 1] = text[i];
			}
			text[terminal->x] = ch;

			terminal->x++;
			check_right_update(terminal);
			move_cursor_to_tab(terminal);

			print_line(terminal, terminal->y);
		}
		break;

		case ENTER_KEYCODE1:
		if (text[0] == ':')
		{
			char* ptr = &text[1];
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

				char* fname = malloc(sizeof(char) * LINE_SIZE);
				if (fname == NULL)
				{
					log_error("malloc failed in terminal_mode\n");
					return;
				}
				int i;
				for (i = 0; ptr[i] != '\0'; i++)
				{
					fname[i] = ptr[i];
				}
				fname[i] = '\0';

				active_tab = make_tab(fname);
				if (active_tab == NULL)
				{
					log_error("make_tab failed in terminal_mode\n");
					active_tab = (Tab*) get_elt(tabs, active_tab_index);
					if (active_tab == NULL)
					{
						log_error("active_tab_index references NULL element in tabs linked list\n");
					}
					return;
				}
				for (int i = 0; i < tabs->size; i++)
				{
					Tab* t = (Tab*) get_elt(tabs, i);
					if (t == NULL)
					{
						log_error("found NULL tab in tabs in terminal_mode\n");
						continue;
					}
					t->z_index_changes_saved++;
				}
				add(tabs, active_tab, tabs->size);
				active_tab_index = tabs->size - 1;
				print_screen();
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
				if (active_tab == NULL)
				{
					log_error("found NULL tab in tabs in terminal_mode\n");
					Node* n = get_node(tabs, active_tab_index);
					if (n != NULL)
					{
						n->elt = make_tab(NULL);
						if (n->elt != NULL)
						{
							active_tab = (Tab*) n->elt;
						}
						else
						{
							log_error("active_tab_index references NULL element in tabs linked list, make_tab failing (terminal_mode)\n");
							return;
						}
					}
					else
					{
						log_error("active_tab_index references NULL element in tabs linked list, make_tab failing (terminal_mode)\n");
						return;
					}
				}
				for (int i = 0; i < tabs->size; i++)
				{
					Tab* t = (Tab*) get_elt(tabs, i);
					if (t == NULL)
					{
						log_error("found NULL tab in tabs in terminal_mode\n");
						continue;
					}
					t->z_index_changes_saved++;
				}
				active_tab->z_index_changes_saved &= CHANGES_SAVED;
				print_screen();
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
				if (active_tab == NULL)
				{
					log_error("found NULL tab in tabs in terminal_mode\n");
					Node* n = get_node(tabs, active_tab_index);
					if (n != NULL)
					{
						n->elt = make_tab(NULL);
						if (n->elt != NULL)
						{
							active_tab = (Tab*) n->elt;
						}
						else
						{
							log_error("active_tab_index references NULL element in tabs linked list, make_tab failing (terminal_mode)\n");
							return;
						}
					}
					else
					{
						log_error("active_tab_index references NULL element in tabs linked list, make_tab failing (terminal_mode)\n");
						return;
					}
				}
				for (int i = 0; i < tabs->size; i++)
				{
					Tab* t = (Tab*) get_elt(tabs, i);
					if (t == NULL)
					{
						log_error("found NULL tab in tabs in terminal_mode\n");
						continue;
					}
					t->z_index_changes_saved++;
				}
				active_tab->z_index_changes_saved &= CHANGES_SAVED;
				print_screen();
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
				if (active_tab == NULL)
				{
					log_error("found NULL tab in tabs in terminal_mode\n");
					Node* n = get_node(tabs, active_tab_index);
					if (n != NULL)
					{
						n->elt = make_tab(NULL);
						if (n->elt != NULL)
						{
							active_tab = (Tab*) n->elt;
						}
						else
						{
							log_error("active_tab_index references NULL element in tabs linked list, make_tab failing (terminal_mode)\n");
							return;
						}
					}
					else
					{
						log_error("active_tab_index references NULL element in tabs linked list, make_tab failing (terminal_mode)\n");
						return;
					}
				}
				for (int i = 0; i < tabs->size; i++)
				{
					Tab* t = (Tab*) get_elt(tabs, i);
					if (t == NULL)
					{
						log_error("found NULL tab in tabs in terminal_mode\n");
						continue;
					}
					t->z_index_changes_saved++;
				}
				active_tab->z_index_changes_saved &= CHANGES_SAVED;
				print_screen();
			}
			else if (!strcmp(ptr, "rs"))
			{
				int amount; 

				int* num_to_change1 = NULL;
				int sign1;

				int* num_to_change2 = NULL;
				int sign2;

				// for making sure the new dimensions still fit on the screen
				int upper_bound;

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
					upper_bound = height - 2;
				}
				else if (!strcmp(ptr, "bottom"))
				{
					num_to_change1 = &active_tab->height;
					upper_bound = height - 2;
				}
				else if (!strcmp(ptr, "left"))
				{
					num_to_change1 = &active_tab->xpos;
					num_to_change2 = &active_tab->width;
					upper_bound = width - 1;
				}
				else if (!strcmp(ptr, "right"))
				{
					num_to_change1 = &active_tab->width;
					upper_bound = width - 1;
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
					if (num_to_change2 != NULL)
					{
						if (*num_to_change1 + sign1 * amount + *num_to_change2 + sign2 * amount <= upper_bound && *num_to_change1 + sign1 * amount >= 0 && *num_to_change2 + sign2 * amount >= 0)
						{
							*num_to_change1 = *num_to_change1 + sign1 * amount;
							*num_to_change2 = *num_to_change2 + sign2 * amount;
						}
						else
						{
							print_message("Resize would cause tab to go off screen");
							make_input_line();
							return;
						}
					}
					else
					{
						if (upper_bound == width - 1)
						{
							if (active_tab->width + sign1 * amount + active_tab->xpos <= upper_bound && active_tab->width + sign1 * amount >= 0)
							{
								active_tab->width = active_tab->width + sign1 * amount;
							}
							else
							{
								print_message("Resize would cause tab to go off screen");
								make_input_line();
								return;
							}
						}
						else
						{
							if (active_tab->height + sign1 * amount + active_tab->ypos <= upper_bound && active_tab->height + sign1 * amount >= 0)
							{
								active_tab->height = active_tab->height + sign1 * amount;
							}
							else
							{
								print_message("Resize would cause tab to go off screen");
								make_input_line();
								return;
							}
						}
					}
				}

				print_screen();
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

				print_screen();
			}
			else if (!strcmp(ptr, "q"))
			{
				if (active_tab->z_index_changes_saved & CHANGES_SAVED)
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
							print_screen();
						}
					}
					else
					{
						active_tab = (Tab*) get_elt(tabs, active_tab_index + 1);
						free_tab((Tab*) rm(tabs, active_tab_index));
						print_screen();
					}

					if (active_tab == NULL)
					{
						log_error("active_tab_index referencing NULL element in tabs linked list in terminal_mode\n");
						active_tab = make_tab(NULL);
						if (active_tab == NULL)
						{
							log_error("make_tab failed in terminal_mode\n");
						}
						else
						{
							add(tabs, active_tab, active_tab_index);
						}
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
						return;
					}
					else
					{
						active_tab = (Tab*) get_elt(tabs, active_tab_index - 1);
						free_tab((Tab*) rm(tabs, active_tab_index));
						active_tab_index--;
						print_screen();
					}
				}
				else
				{
					active_tab = (Tab*) get_elt(tabs, active_tab_index + 1);
					free_tab((Tab*) rm(tabs, active_tab_index));
					print_screen();
				}

				if (active_tab == NULL)
				{
					log_error("active_tab_index referencing NULL element in tabs linked list in terminal_mode\n");
					active_tab = make_tab(NULL);
					if (active_tab == NULL)
					{
						log_error("make_tab failed in terminal_mode\n");
					}
					else
					{
						add(tabs, active_tab, active_tab_index);
					}
				}
			}
			else if (!strcmp(ptr, "w"))
			{
				if (active_tab->fname == NULL)
				{
					char* fname = malloc(sizeof(char) * 10);
					if (fname == NULL)
					{
						log_error("malloc failed in terminal_mode\n");
						return;
					}
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
				if (f == NULL)
				{
					log_error("failed to open file to write to in terminal_mode\n");
					return;
				}
				for (int i = 0; i < active_tab->lines->size; i++)
				{
					fprintf(f, "%s\n", ((Line*) get_elt(active_tab->lines, i))->text);
				}
				fclose(f);
				active_tab->z_index_changes_saved |= CHANGES_SAVED;
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
			for (; text[i] != '\0'; i++) {}
			text[i] = '\n';
			text[i + 1] = '\0';
			write(master_fd, text, i + 2);
			text[0] = '\0';
			terminal->x = 0;
			check_bottom_update(terminal);
			move_cursor_to_tab(terminal);
		}
		break;

		case BACKSPACE_KEYCODE2:
		if (terminal->x > 0)
		{
			for (int i = terminal->x - 1; text[i] != '\0'; i++)
			{
				text[i] = text[i + 1];
			}

			terminal->x--;
			check_left_update(terminal);
			move_cursor_to_tab(terminal);

			print_line(terminal, terminal->y);
		}
		break;

		case ESCAPE_KEYCODE:
		print_screen();
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
		if (listener_buf == NULL)
		{
			log_error("malloc failed in forkpty thread\n");
			continue;
		}
		int bytes_read = read(master_fd, listener_buf, LINE_SIZE);
		listener_buf[bytes_read] = '\0';
		if (bytes_read > 0)
		{
			sem_wait(&sem);
			int index = 0;
			int i = 0;
			for (; i < bytes_read; i++)
			{
				if (listener_buf[i] == '\n')
				{
					char* line = malloc(sizeof(char) * (i + 1 - index));
					if (line == NULL)
					{
						log_error("malloc failed in forkpty thread\n");
						continue;
					}

					int chars_skipped = 0;
					int j = 0;
					unsigned char sequence = 0;
					while (j < (i - index - chars_skipped))
					{
						if (sequence == 0 && listener_buf[index + j + chars_skipped] != ESCAPE_KEYCODE && listener_buf[index + j + chars_skipped] != '\r')
						{
							line[j] = listener_buf[index + j + chars_skipped];
							j++;
						}
						else if (sequence & CSI_ESC)
						{
							if (listener_buf[index + j + chars_skipped] >='@' && listener_buf[index + j + chars_skipped] <= 'z')
							{
								sequence = 0;
							}
							chars_skipped++;
						}
						else if (sequence & OSC_ESC)
						{
							if (listener_buf[index + j + chars_skipped] == '\a')
							{
								sequence = 0;
							}
							chars_skipped++;
						}
						else if (sequence & SC_ESC)
						{
							sequence = 0;
							chars_skipped++;
						}
						else if (listener_buf[index + j + chars_skipped] == '\r')
						{
							chars_skipped++;
						}
						else
						{
							if (listener_buf[index + j + chars_skipped + 1] == '[')
							{
								sequence = CSI_ESC;
								chars_skipped += 2;
							}
							else if (listener_buf[index + j + chars_skipped + 1] == ']')
							{
								sequence = OSC_ESC;
								chars_skipped += 2;
							}
							else
							{
								sequence = SC_ESC;
								chars_skipped++;
							}
						}
					}
					line[j] = '\0';
					index = i + 1;

					Line* l = malloc(sizeof(Line));
					if (l == NULL)
					{
						log_error("malloc failed in forkpty thread\n");
						free(line);
						continue;
					}
					l->text = line;
					l->color_indices = NULL;
					add(terminal->lines, l, terminal->lines->size - 1);
					terminal->y++;
				}
			}

			char* line = malloc(sizeof(char) * (i + 1 - index));
			if (line == NULL)
			{
				log_error("malloc failed in forkpty thread\n");
				free(listener_buf);
				listener_buf = NULL;
				continue;
			}

			int chars_skipped = 0;
			int j = 0;
			unsigned char sequence = 0;
			while (j < (i - index - chars_skipped))
			{
				if (sequence == 0 && listener_buf[index + j + chars_skipped] != ESCAPE_KEYCODE && listener_buf[index + j + chars_skipped] != '\r')
				{
					line[j] = listener_buf[index + j + chars_skipped];
					j++;
				}
				else if (sequence & CSI_ESC)
				{
					if (listener_buf[index + j + chars_skipped] >='@' && listener_buf[index + j + chars_skipped] <= 'z')
					{
						sequence = 0;
					}
					chars_skipped++;
				}
				else if (sequence & OSC_ESC)
				{
					if (listener_buf[index + j + chars_skipped] == '\a')
					{
						sequence = 0;
					}
					chars_skipped++;
				}
				else if (sequence & SC_ESC)
				{
					sequence = 0;
					chars_skipped++;
				}
				else if (listener_buf[index + j + chars_skipped] == '\r')
				{
					chars_skipped++;
				}
				else
				{
					if (listener_buf[index + j + chars_skipped + 1] == '[')
					{
						sequence = CSI_ESC;
						chars_skipped += 2;
					}
					else if (listener_buf[index + j + chars_skipped + 1] == ']')
					{
						sequence = OSC_ESC;
						chars_skipped += 2;
					}
					else
					{
						sequence = SC_ESC;
						chars_skipped++;
					}
				}
			}
			line[j] = '\0';

			Line* l = malloc(sizeof(Line));
			if (l == NULL)
			{
				log_error("malloc failed in forkpty thread\n");
				free(listener_buf);
				free(line);
				listener_buf = NULL;
				continue;
			}

			l->text = line;
			l->color_indices = NULL;
			add(terminal->lines, l, terminal->lines->size - 1);
			terminal->y++;

			free(listener_buf);
			listener_buf = NULL;

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
