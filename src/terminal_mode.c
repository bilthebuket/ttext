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
	GapBuffer* gb = gb_create(NULL, -1);
	if (gb == NULL)
	{
		log_error("gb_create failed in make_input_line\n");
		return;
	}

	Line* l = malloc(sizeof(Line));
	if (l == NULL)
	{
		log_error("malloc failed in make_input_line\n");
		return;
	}
	l->gb = gb;
	l->color_indices = NULL;
	add(terminal->lines, l, terminal->lines->size);
	gb_goto(gb, 0);
	terminal->x = 0;
	terminal->y++;
	check_bottom_update(terminal);
	move_cursor_to_tab(terminal);
}

Tab* terminal_mode(Tab* t, int ch)
{
	Line* line = (Line*) get_elt(terminal->lines, terminal->y);
	if (line == NULL)
	{
		log_error("NULL line in terminal_mode\n");
		return;
	}
	GapBuffer* gb = line->gb;
	if (gb == NULL)
	{
		log_error("NULL gb in terminal_mode\n");
		return;
	}

	Tab* r = t;
	switch (ch)
	{
		default:
		gb_put(gb, ch);
		terminal->x++;
		check_right_update(terminal);
		move_cursor_to_tab(terminal);
		print_line(terminal, terminal->y);
		break;

		case ENTER_KEYCODE1:
		if (gb_get(gb, 0) == ':')
		{
			int start_index = 1;
			int end_index = 1;
			bool only_one_arg = false;

			for (; gb_get(gb, end_index) != ' ' && gb_get(gb, end_index) != '\0'; end_index++) {}

			if (gb_get(gb, end_index) == '\0')
			{
				only_one_arg = true;
			}

			if (!gb_strcmp(gb, start_index, end_index, "tabnew"))
			{
				if (only_one_arg)
				{
					print_message("Please pass filename as argument");
					make_input_line();
					return;
				}

				start_index = end_index + 1;

				char* fname = malloc(sizeof(char) * LINE_SIZE);
				if (fname == NULL)
				{
					log_error("malloc failed in terminal_mode\n");
					return;
				}
				int i;
				for (i = start_index; gb_get(gb, i) != '\0'; i++)
				{
					fname[i - start_index] = gb_get(gb, i);
				}
				fname[i - start_index] = '\0';

				r = make_tab(fname);
				if (r == NULL)
				{
					log_error("make_tab failed in terminal_mode\n");
					r = (Tab*) get_elt(tabs, active_tab_index);
					if (active_tab == NULL)
					{
						log_error("active_tab_index references NULL element in tabs linked list\n");
					}
					return;
				}

				r->tab_num_flags &= FLAG_BITS;
				r->tab_num_flags |= tabs->size;
				add(tabs, r, tabs->size);
				active_tab_index = tabs->size - 1;
				print_screen();
			}
			else if (!gb_strcmp(gb, start_index, end_index, "tabn"))
			{
				int tab_num = t->tab_num_flags & TAB_NUM_BITS;
				if (tab_num == tabs->size - 1)
				{
					tab_num = 0;
				}
				else
				{
					tab_num++;
				}

				for (int i = 0; i < tabs->size; i++)
				{
					Tab* t = (Tab*) get_elt(tabs, i);
					if (t == NULL)
					{
						log_error("found NULL tab in tabs in terminal_mode\n");
						Node* n = get_node(tabs, i);
						if (n != NULL)
						{
							n->elt = make_tab(NULL);
							if (n->elt != NULL)
							{
								r = (Tab*) n->elt;
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
					else if ((t->tab_num_flags & TAB_NUM_BITS) == tab_num)
					{
						r = t;
						active_tab_index = i;
						break;
					}
				}

				rm(tabs, active_tab_index);
				add(tabs, r, tabs->size);
				active_tab_index = tabs->size - 1;
				print_screen();
			}
			else if (!gb_strcmp(gb, start_index, end_index, "tabp"))
			{
				int tab_num = t->tab_num_flags & TAB_NUM_BITS;
				if (tab_num == 0)
				{
					tab_num = tabs->size - 1;
				}
				else
				{
					tab_num--;
				}

				for (int i = 0; i < tabs->size; i++)
				{
					Tab* t = (Tab*) get_elt(tabs, i);
					if (t == NULL)
					{
						log_error("found NULL tab in tabs in terminal_mode\n");
						Node* n = get_node(tabs, i);
						if (n != NULL)
						{
							n->elt = make_tab(NULL);
							if (n->elt != NULL)
							{
								r = (Tab*) n->elt;
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
					else if ((t->tab_num_flags & TAB_NUM_BITS) == tab_num)
					{
						r = t;
						active_tab_index = i;
						break;
					}
				}

				rm(tabs, active_tab_index);
				add(tabs, r, tabs->size);
				active_tab_index = tabs->size - 1;
				print_screen();
			}
			else if (!gb_strcmp(gb, start_index, end_index, "tab"))
			{
				if (only_one_arg)
				{
					print_message("Please pass the index of the tab to switch to");
					make_input_line();
					return;
				}

				start_index = end_index + 1;
				end_index = gb->num_chars - 1;
				int tab_num = gb_atoi(gb, start_index, end_index);

				if (tab_num < 0 || tab_num >= tabs->size)
				{
					print_message("tab number invalid");
					make_input_line();
					return;
				}

				for (int i = 0; i < tabs->size; i++)
				{
					Tab* t = (Tab*) get_elt(tabs, i);
					if (t == NULL)
					{
						log_error("found NULL tab in tabs in terminal_mode\n");
						Node* n = get_node(tabs, i);
						if (n != NULL)
						{
							n->elt = make_tab(NULL);
							if (n->elt != NULL)
							{
								r = (Tab*) n->elt;
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
					else if ((t->tab_num_flags & TAB_NUM_BITS) == tab_num)
					{
						r = t;
						active_tab_index = i;
						break;
					}
				}

				rm(tabs, active_tab_index);
				add(tabs, r, tabs->size);
				active_tab_index = tabs->size - 1;
				print_screen();
			}
			else if (!gb_strcmp(gb, start_index, end_index, "rs"))
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

				start_index = end_index + 1;
				end_index++;
				for (; gb_get(gb, end_index) != '\0' && gb_get(gb, end_index) != ' '; end_index++) {}

				if (gb_get(gb, end_index) == '\0')
				{
					print_message("Usage: :rs <top/bottom/left/right> <add/sub> <amount>");
					make_input_line();
					return;
				}

				if (!gb_strcmp(gb, start_index, end_index, "top"))
				{
					num_to_change1 = &t->ypos;
					num_to_change2 = &t->height;
					upper_bound = height - 2;
				}
				else if (!gb_strcmp(gb, start_index, end_index, "bottom"))
				{
					num_to_change1 = &t->height;
					upper_bound = height - 2;
				}
				else if (!gb_strcmp(gb, start_index, end_index, "left"))
				{
					num_to_change1 = &t->xpos;
					num_to_change2 = &t->width;
					upper_bound = width - 1;
				}
				else if (!gb_strcmp(gb, start_index, end_index, "right"))
				{
					num_to_change1 = &t->width;
					upper_bound = width - 1;
				}

				start_index = end_index + 1;
				end_index++;
				for (; gb_get(gb, end_index) != '\0' && gb_get(gb, end_index) != ' '; end_index++) {}

				if (gb_get(gb, end_index) == '\0')
				{
					print_message("Usage: :rs <top/bottom/left/right> <add/sub> <amount>");
					make_input_line();
					return;
				}

				if (!gb_strcmp(gb, start_index, end_index, "add"))
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
				else if (!gb_strcmp(gb, start_index, end_index, "sub"))
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

				start_index = end_index + 1;;
				end_index = gb->num_chars - 1;
				amount = gb_atoi(gb, start_index, end_index);

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
							if (t->width + sign1 * amount + t->xpos <= upper_bound && t->width + sign1 * amount >= 0)
							{
								t->width = t->width + sign1 * amount;
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
							if (t->height + sign1 * amount + t->ypos <= upper_bound && t->height + sign1 * amount >= 0)
							{
								t->height = t->height + sign1 * amount;
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
			else if (!gb_strcmp(gb, start_index, end_index, "mv"))
			{
				if (only_one_arg)
				{
					print_message("Ussage: :mv <left/right/up/down> <amount>");
					make_input_line();
					return;
				}

				int* num_to_change = NULL;
				int bound = -1;
				int sign;
				int amount;

				start_index = end_index + 1;
				end_index++;
				for (; gb_get(gb, end_index) != ' ' && gb_get(gb, end_index) != '\0'; end_index++) {}
				if (gb_get(gb, end_index) == '\0')
				{
					print_message("Ussage: :mv <left/right/up/down> <amount>");
					make_input_line();
					return;
				}

				if (!gb_strcmp(gb, start_index, end_index, "left"))
				{
					num_to_change = &t->xpos;
					sign = -1;
					bound = 0;
				}
				else if (!gb_strcmp(gb, start_index, end_index, "right"))
				{
					num_to_change = &t->xpos;
					sign = 1;
					bound = width - t->width;
				}
				else if (!gb_strcmp(gb, start_index, end_index, "up"))
				{
					num_to_change = &t->ypos;
					sign = -1;
					bound = 0;
				}
				else if (!gb_strcmp(gb, start_index, end_index, "down"))
				{
					num_to_change = &t->ypos;
					sign = 1;
					bound = height - t->height - 2;
				}

				start_index = end_index + 1;
				end_index = gb->num_chars - 1;
				amount = gb_atoi(gb, start_index, end_index);

				if (num_to_change != NULL)
				{
					if (sign == -1)
					{
						if (*num_to_change + amount * sign >= bound)
						{
							*num_to_change = *num_to_change + amount * sign;
							print_screen();
						}
						else
						{
							print_message("Move would cause tab to go off screen");
						}
					}
					else
					{
						if (*num_to_change + amount * sign <= bound)
						{
							*num_to_change = *num_to_change + amount * sign;
							print_screen();
						}
						else
						{
							print_message("Move would cause tab to go off screen");
						}
					}
				}
			}
			else if (!gb_strcmp(gb, start_index, end_index, "q"))
			{
				if (t->tab_num_flags & CHANGES_SAVED)
				{
					if (t== tabs->size - 1)
					{
						if (tabs->size == 1)
						{
							terminate = true;
						}
						else
						{
							int tab_num = t->tab_num_flags & TAB_NUM_BITS;
							for (int i = 0; i < tabs->size; i++)
							{
								Tab* t = get_elt(tabs, i);
								if (t != NULL && (t->tab_num_flags & TAB_NUM_BITS) > tab_num)
								{
									t->tab_num_flags--;
								}
							}
							r = (Tab*) get_elt(tabs, active_tab_index - 1);
							free_tab((Tab*) rm(tabs, active_tab_index));
							active_tab_index--;
							print_screen();
						}
					}
					else
					{
						int tab_num = t->tab_num_flags & TAB_NUM_BITS;
						for (int i = 0; i < tabs->size; i++)
						{
							Tab* t = get_elt(tabs, i);
							if (t != NULL && (t->tab_num_flags & TAB_NUM_BITS) > tab_num)
							{
								t->tab_num_flags--;
							}
						}
						r = (Tab*) get_elt(tabs, active_tab_index + 1);
						free_tab((Tab*) rm(tabs, active_tab_index));
						print_screen();
					}

					if (r == NULL)
					{
						log_error("active_tab_index referencing NULL element in tabs linked list in terminal_mode\n");
						r = make_tab(NULL);
						if (r == NULL)
						{
							log_error("make_tab failed in terminal_mode\n");
						}
						else
						{
							add(tabs, r, active_tab_index);
						}
					}
				}
			}
			else if (!gb_strcmp(gb, start_index, end_index, "q!"))
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
						int tab_num = t->tab_num_flags & TAB_NUM_BITS;
						for (int i = 0; i < tabs->size; i++)
						{
							Tab* t = get_elt(tabs, i);
							if (t != NULL && (t->tab_num_flags & TAB_NUM_BITS) > tab_num)
							{
								t->tab_num_flags--;
							}
						}
						r = (Tab*) get_elt(tabs, active_tab_index - 1);
						free_tab((Tab*) rm(tabs, active_tab_index));
						active_tab_index--;
						print_screen();
					}
				}
				else
				{
					int tab_num = t->tab_num_flags & TAB_NUM_BITS;
					for (int i = 0; i < tabs->size; i++)
					{
						Tab* t = get_elt(tabs, i);
						if (t != NULL && (t->tab_num_flags & TAB_NUM_BITS) > tab_num)
						{
							t->tab_num_flags--;
						}
					}
					r = (Tab*) get_elt(tabs, active_tab_index + 1);
					free_tab((Tab*) rm(tabs, active_tab_index));
					print_screen();
				}

				if (r == NULL)
				{
					log_error("active_tab_index referencing NULL element in tabs linked list in terminal_mode\n");
					r = make_tab(NULL);
					if (r == NULL)
					{
						log_error("make_tab failed in terminal_mode\n");
					}
					else
					{
						add(tabs, r, active_tab_index);
					}
				}
			}
			else if (!gb_strcmp(gb, start_index, end_index, "w"))
			{
				if (t->fname == NULL)
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
					t->fname = fname;
				}

				FILE* f = fopen(t->fname, "w");
				if (f == NULL)
				{
					log_error("failed to open file to write to in terminal_mode\n");
					return;
				}
				for (int i = 0; i < t->lines->size; i++)
				{
					Line* l = (Line*) get_elt(t->lines, i);
					if (l != NULL)
					{
						GapBuffer* gb = l->gb;
						if (gb != NULL)
						{
							int store = gb->gap_index;
							gb_goto(gb, gb->num_chars - 1);
							fprintf(f, "%s\n", gb->text);
							gb_goto(gb, store);
						}
					}
				}
				fclose(f);
				t->tab_num_flags |= CHANGES_SAVED;
			}
			else if (!gb_strcmp(gb, start_index, end_index, "findreplace"))
			{

			}

			make_input_line();
			print_tab(terminal);
		}
		else
		{
			gb_goto(gb, gb->num_chars - 1);
			gb_put(gb, '\n');
			write(master_fd, gb->text, gb->num_chars - 1);
			gb_goto(gb, 0);
			while (gb->num_chars > 1)
			{
				gb_rm(gb);
			}
			terminal->x = 0;
			check_bottom_update(terminal);
			move_cursor_to_tab(terminal);
		}
		break;

		case BACKSPACE_KEYCODE2:
		if (terminal->x > 0)
		{
			gb_goto(gb, terminal->x - 1);
			gb_rm(gb);
			terminal->x--;
			check_left_update(terminal);
			move_cursor_to_tab(terminal);
			print_line(terminal, terminal->y);
		}
		break;

		case ESCAPE_KEYCODE:
		print_screen();
		move_cursor_to_tab(t);
		mode = &normal_mode;
		break;
	}

	return r;
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
					int size = i + 1 - index;
					char* line = malloc(sizeof(char) * size);
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
					l->gb = gb_create(line, -1);
					l->color_indices = NULL;
					add(terminal->lines, l, terminal->lines->size - 1);
					terminal->y++;
				}
			}

			int size = i + 1 - index;
			char* line = malloc(sizeof(char) * size);
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

			l->gb = gb_create(line, -1);
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
