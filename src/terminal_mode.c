#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ncurses.h>
#include <pthread.h>
#include <pty.h>
#include <sys/wait.h>
#include <signal.h>
#include "terminal_mode.h"
#include "normal_mode/normal_mode.h"
#include "global.h"
#include "io_tools.h"
#include "line.h"
#include "finder.h"
#include "signature.h"
#include "snake.h"

static Tab* terminal;
static char* listener_buf = NULL;
static int slave_pid;
static int master_fd;
static pthread_t listener;

static void (*execute_char[NUM_CHARS])(EditorState*, int);

void print_terminal(void)
{
	print_tab(terminal);
}

void move_cursor_to_terminal(void)
{
	move_cursor_to_tab(terminal);
}

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
	ll_insert(terminal->lines, l, terminal->lines->size);
	gb_goto(gb, 0);
	terminal->x = 0;
	terminal->y++;
	check_bottom_update(terminal);
	move_cursor_to_tab(terminal);
}

static void handle_default(EditorState* es, int ch)
{
	(void) es;
	Line* line = (Line*) ll_get_elt(terminal->lines, terminal->y);
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

	gb_insert(gb, ch);
	terminal->x++;
	check_right_update(terminal);
	move_cursor_to_tab(terminal);
	print_line(terminal, terminal->y);
}

static void handle_enter(EditorState* es, int ch)
{
	(void) ch;
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}
	Line* line = (Line*) ll_get_elt(terminal->lines, terminal->y);
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

		if (!gb_strcmp(gb, start_index, end_index, "find"))
		{
			if (only_one_arg)
			{
				print_message("Please pass a string to look for");
				make_input_line();
				return;
			}

			start_index = end_index + 1;
			end_index++;
			for (; gb_get(gb, end_index) != '\0'; end_index++) {}

			char* string_to_look_for = malloc(sizeof(char) * (end_index - start_index + 1));
			if (string_to_look_for == NULL)
			{
				return;
			}

			for (int i = start_index; i <= end_index; i++)
			{
				string_to_look_for[i - start_index] = gb_get(gb, i);
			}

			finder_free(es->finder);
			es->finder = finder_create(t->pt, string_to_look_for);
			find_next(t, es->finder);
			es->flags &= ~UPDATE_FINDER_FLAG;
		}
		else if (!gb_strcmp(gb, start_index, end_index, "tabnew"))
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

			Tab* new_tab = tab_create(fname);
			if (new_tab == NULL)
			{
				log_error("tab_create failed in terminal_mode\n");
				return;
			}

			new_tab->tab_num_flags &= FLAG_BITS;
			new_tab->tab_num_flags |= es->tabs->size;
			ll_insert(es->tabs, new_tab, es->tabs->size);
			es->active_tab_index = es->tabs->size - 1;
			es->active_tab = new_tab;
			print_screen(es);
			es->flags |= UPDATE_FINDER_FLAG;
		}
		else if (!gb_strcmp(gb, start_index, end_index, "tabn"))
		{
			int tab_num = t->tab_num_flags & TAB_NUM_BITS;
			if (tab_num == es->tabs->size - 1)
			{
				tab_num = 0;
			}
			else
			{
				tab_num++;
			}

			for (int i = 0; i < es->tabs->size; i++)
			{
				Tab* t = (Tab*) ll_get_elt(es->tabs, i);
				if (t == NULL)
				{
					log_error("found NULL tab in es->tabs in terminal_mode\n");
					Node* n = ll_get_node(es->tabs, i);
					if (n != NULL)
					{
						n->elt = tab_create(NULL);
						if (n->elt == NULL)
						{
							log_error("tab_create failing (terminal_mode)\n");
							return;
						}
					}
					else
					{
						log_error("found NULL node is es->tabs (terminal_mode)\n");
						return;
					}
				}
				else if ((t->tab_num_flags & TAB_NUM_BITS) == tab_num)
				{
					es->active_tab = t;
					es->active_tab_index = i;
					break;
				}
			}

			ll_rm(es->tabs, es->active_tab_index);
			ll_insert(es->tabs, es->active_tab, es->tabs->size);
			es->active_tab_index = es->tabs->size - 1;
			print_screen(es);
			es->flags |= UPDATE_FINDER_FLAG;
		}
		else if (!gb_strcmp(gb, start_index, end_index, "tabp"))
		{
			int tab_num = t->tab_num_flags & TAB_NUM_BITS;
			if (tab_num == 0)
			{
				tab_num = es->tabs->size - 1;
			}
			else
			{
				tab_num--;
			}

			for (int i = 0; i < es->tabs->size; i++)
			{
				Tab* t = (Tab*) ll_get_elt(es->tabs, i);
				if (t == NULL)
				{
					log_error("found NULL tab in es->tabs in terminal_mode\n");
					Node* n = ll_get_node(es->tabs, i);
					if (n != NULL)
					{
						n->elt = tab_create(NULL);
						if (n->elt == NULL)
						{
							log_error("tab_create failing (terminal_mode)\n");
							return;
						}
					}
					else
					{
						log_error("found NULL node is es->tabs (terminal_mode)\n");
						return;
					}
				}
				else if ((t->tab_num_flags & TAB_NUM_BITS) == tab_num)
				{
					es->active_tab = t;
					es->active_tab_index = i;
					break;
				}
			}

			ll_rm(es->tabs, es->active_tab_index);
			ll_insert(es->tabs, es->active_tab, es->tabs->size);
			es->active_tab_index = es->tabs->size - 1;
			print_screen(es);
			es->flags |= UPDATE_FINDER_FLAG;
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
			int tab_num = gb_atoi(gb, start_index, end_index, false);

			if (tab_num < 0 || tab_num >= es->tabs->size)
			{
				print_message("tab number invalid");
				make_input_line();
				return;
			}

			for (int i = 0; i < es->tabs->size; i++)
			{
				Tab* t = (Tab*) ll_get_elt(es->tabs, i);
				if (t == NULL)
				{
					log_error("found NULL tab in es->tabs in terminal_mode\n");
					Node* n = ll_get_node(es->tabs, i);
					if (n != NULL)
					{
						n->elt = tab_create(NULL);
						if (n->elt == NULL)
						{
							log_error("tab_create failing (terminal_mode)\n");
							return;
						}
					}
					else
					{
						log_error("found NULL node is es->tabs (terminal_mode)\n");
						return;
					}
				}
				else if ((t->tab_num_flags & TAB_NUM_BITS) == tab_num)
				{
					es->active_tab = t;
					es->active_tab_index = i;
					break;
				}
			}

			ll_rm(es->tabs, es->active_tab_index);
			ll_insert(es->tabs, es->active_tab, es->tabs->size);
			es->active_tab_index = es->tabs->size - 1;
			print_screen(es);
			es->flags |= UPDATE_FINDER_FLAG;
		}
		else if (!gb_strcmp(gb, start_index, end_index, "rs"))
		{
			int amount; 

			int* num_to_change1 = NULL;
			int sign1;
			int store1;

			int* num_to_change2 = NULL;
			int sign2;
			int store2;


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
				store1 = t->ypos;
				store2 = t->height;
			}
			else if (!gb_strcmp(gb, start_index, end_index, "bottom"))
			{
				num_to_change1 = &t->height;
				store1 = t->height;
			}
			else if (!gb_strcmp(gb, start_index, end_index, "left"))
			{
				num_to_change1 = &t->xpos;
				num_to_change2 = &t->width;
				store1 = t->xpos;
				store2 = t->width;
			}
			else if (!gb_strcmp(gb, start_index, end_index, "right"))
			{
				num_to_change1 = &t->width;
				store1 = t->width;
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
			amount = gb_atoi(gb, start_index, end_index, true);

			if (amount < 0)
			{
				print_message("An error has occured (potential fix: enter a non negative value)");
				make_input_line();
				return;
			}

			if (num_to_change1 != NULL)
			{
				if (num_to_change2 != NULL)
				{
					*num_to_change1 = *num_to_change1 + sign1 * amount;
					*num_to_change2 = *num_to_change2 + sign2 * amount;
					if (!is_tab_on_screen(t))
					{
						*num_to_change1 = store1;
						*num_to_change2 = store2;
						print_message("Resize would cause tab to go off screen");
						make_input_line();
						return;
					}
				}
				else
				{
					*num_to_change1 = *num_to_change1 + sign1 * amount;
					if (!is_tab_on_screen(t))
					{
						*num_to_change1 = store1;
						print_message("Resize would cause tab to go off screen");
						make_input_line();
						return;
					}
				}
			}

			print_screen(es);
		}
		else if (!gb_strcmp(gb, start_index, end_index, "mv"))
		{
			if (only_one_arg)
			{
				print_message("Usage: :mv <left/right/up/down> <amount>");
				make_input_line();
				return;
			}

			int* num_to_change = NULL;
			int sign;
			int amount;
			int store;

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
				store = t->xpos;
			}
			else if (!gb_strcmp(gb, start_index, end_index, "right"))
			{
				num_to_change = &t->xpos;
				sign = 1;
				store = t->xpos;
			}
			else if (!gb_strcmp(gb, start_index, end_index, "up"))
			{
				num_to_change = &t->ypos;
				sign = -1;
				store = t->ypos;
			}
			else if (!gb_strcmp(gb, start_index, end_index, "down"))
			{
				num_to_change = &t->ypos;
				sign = 1;
				store = t->ypos;
			}

			start_index = end_index + 1;
			end_index = gb->num_chars - 1;
			amount = gb_atoi(gb, start_index, end_index, true);

			if (amount < 0)
			{
				print_message("An error has occured (potential fix: enter a non negative value)");
				make_input_line();
				return;
			}

			if (num_to_change != NULL)
			{
				*num_to_change = *num_to_change + amount * sign;
				if (is_tab_on_screen(t))
				{
					print_screen(es);
				}
				else
				{
					*num_to_change = store;
					print_message("Move would cause tab to go off screen");
				}
			}
		}
		else if (!gb_strcmp(gb, start_index, end_index, "q"))
		{
			if (t->tab_num_flags & CHANGES_SAVED)
			{
				if (es->active_tab_index == es->tabs->size - 1)
				{
					if (es->tabs->size == 1)
					{
						es->flags = TERMINATE_FLAG;
					}
					else
					{
						int tab_num = t->tab_num_flags & TAB_NUM_BITS;
						for (int i = 0; i < es->tabs->size; i++)
						{
							Tab* t = ll_get_elt(es->tabs, i);
							if (t != NULL && (t->tab_num_flags & TAB_NUM_BITS) > tab_num)
							{
								t->tab_num_flags--;
							}
						}
						es->active_tab = (Tab*) ll_get_elt(es->tabs, es->active_tab_index - 1);
						tab_free((Tab*) ll_rm(es->tabs, es->active_tab_index));
						es->active_tab_index--;
						print_screen(es);
						es->flags |= UPDATE_FINDER_FLAG;
					}
				}
				else
				{
					int tab_num = t->tab_num_flags & TAB_NUM_BITS;
					for (int i = 0; i < es->tabs->size; i++)
					{
						Tab* t = ll_get_elt(es->tabs, i);
						if (t != NULL && (t->tab_num_flags & TAB_NUM_BITS) > tab_num)
						{
							t->tab_num_flags--;
						}
					}
					es->active_tab = (Tab*) ll_get_elt(es->tabs, es->active_tab_index + 1);
					tab_free((Tab*) ll_rm(es->tabs, es->active_tab_index));
					print_screen(es);
					es->flags |= UPDATE_FINDER_FLAG;
				}

				if (es->active_tab == NULL)
				{
					log_error("es->active_tab_index referencing NULL element in es->tabs linked list in terminal_mode\n");
					es->active_tab = tab_create(NULL);
					es->flags |= UPDATE_FINDER_FLAG;
					if (es->active_tab == NULL)
					{
						log_error("tab_create failed in terminal_mode\n");
					}
					else
					{
						ll_insert(es->tabs, es->active_tab, es->active_tab_index);
					}
				}
			}
		}
		else if (!gb_strcmp(gb, start_index, end_index, "q!"))
		{
			if (es->active_tab_index == es->tabs->size - 1)
			{
				if (es->tabs->size == 1)
				{
					es->flags = TERMINATE_FLAG;
					return;
				}
				else
				{
					int tab_num = t->tab_num_flags & TAB_NUM_BITS;
					for (int i = 0; i < es->tabs->size; i++)
					{
						Tab* t = ll_get_elt(es->tabs, i);
						if (t != NULL && (t->tab_num_flags & TAB_NUM_BITS) > tab_num)
						{
							t->tab_num_flags--;
						}
					}
					es->active_tab = (Tab*) ll_get_elt(es->tabs, es->active_tab_index - 1);
					tab_free((Tab*) ll_rm(es->tabs, es->active_tab_index));
					es->active_tab_index--;
					print_screen(es);
					es->flags |= UPDATE_FINDER_FLAG;
				}
			}
			else
			{
				int tab_num = t->tab_num_flags & TAB_NUM_BITS;
				for (int i = 0; i < es->tabs->size; i++)
				{
					Tab* t = ll_get_elt(es->tabs, i);
					if (t != NULL && (t->tab_num_flags & TAB_NUM_BITS) > tab_num)
					{
						t->tab_num_flags--;
					}
				}
				es->active_tab = (Tab*) ll_get_elt(es->tabs, es->active_tab_index + 1);
				tab_free((Tab*) ll_rm(es->tabs, es->active_tab_index));
				print_screen(es);
				es->flags |= UPDATE_FINDER_FLAG;
			}

			if (es->active_tab == NULL)
			{
				log_error("es->active_tab_index referencing NULL element in es->tabs linked list in terminal_mode\n");
				es->active_tab = tab_create(NULL);
				es->flags |= UPDATE_FINDER_FLAG;
				if (es->active_tab == NULL)
				{
					log_error("tab_create failed in terminal_mode\n");
				}
				else
				{
					ll_insert(es->tabs, es->active_tab, es->active_tab_index);
				}
			}
		}
		else if (!gb_strcmp(gb, start_index, end_index, "w"))
		{
			if (t->fname == NULL)
			{
				return;
			}

			FILE* f = fopen(t->fname, "w");
			if (f == NULL)
			{
				log_error("failed to open file to write to in terminal_mode\n");
				return;
			}
			PieceIterator pi;
			if (!pt_iterator_init(t->pt, &pi, 0))
			{
				fclose(f);
				return;
			}

			char c = pt_iterate(&pi);
			while (c != '\0')
			{
				fprintf(f, "%c", c);
				c = pt_iterate(&pi);
			}
			fclose(f);
			t->tab_num_flags |= CHANGES_SAVED;
		}
		else if (!gb_strcmp(gb, start_index, end_index, "flookup"))
		{
			if (only_one_arg)
			{
				print_message("usage: :flookup <function_name>");
				return;
			}

			start_index = end_index + 1;
			end_index++;
			for (; gb_get(gb, end_index) != ' ' && gb_get(gb, end_index) != '\0'; end_index++) {}

			char buf[end_index - start_index + 1];
			for (int i = start_index; i < end_index; i++)
			{
				buf[i - start_index] = gb_get(gb, i);
			}
			buf[end_index - start_index] = '\0';

			LinkedList* lst = hm_get(es->signatures, buf, &hash_function, &function_name_equals, NULL);
			gb_goto(gb, gb->num_chars - 1);
			if (lst != NULL)
			{
				while (lst->size > 0)
				{
					Signature* s = (Signature*) ll_rm(lst, 0);
					int len1 = 0;
					for (; s->file_name[len1] != '\0'; len1++) {}
					int len2 = 0;
					for (; s->signature[len2] != '\0'; len2++) {}
					// the 3 covers the ':' ' ' and '\0'
					char* text = malloc(sizeof(char) * (len1 + len2 + 3));
					if (text == NULL)
					{
						continue;
					}

					for (int i = 0; i < len1; i++)
					{
						text[i] = s->file_name[i];
					}
					text[len1] = ':';
					text[len1 + 1] = ' ';
					for (int i = 0; i < len2; i++)
					{
						text[i + len1 + 2] = s->signature[i];
					}
					text[len1 + len2 + 2] = '\0';

					Line* l = malloc(sizeof(Line));
					if (l == NULL)
					{
						free(text);
						continue;
					}

					l->gb = gb_create(text, -1);
					if (l->gb == NULL)
					{
						free(l);
						free(text);
						continue;
					}

					ll_insert(terminal->lines, l, terminal->lines->size);
					terminal->y++;
				}
				ll_free(lst);
			}

		}
		else if (!gb_strcmp(gb, start_index, end_index, "print_signatures"))
		{
			print_all_signatures(es->signatures, stderr);
		}
		else if (!gb_strcmp(gb, start_index, end_index, "snake"))
		{
			snake_execute(t);
		}

		make_input_line();
		print_tab(terminal);
	}
	else
	{
		gb_goto(gb, gb->num_chars - 1);
		gb_insert(gb, '\n');
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
}

static void handle_backspace(EditorState* es, int ch)
{
	(void) es;
	(void) ch;
	Line* line = (Line*) ll_get_elt(terminal->lines, terminal->y);
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

	if (terminal->x > 0)
	{
		gb_goto(gb, terminal->x - 1);
		gb_rm(gb);
		terminal->x--;
		check_left_update(terminal);
		move_cursor_to_tab(terminal);
		print_line(terminal, terminal->y);
	}
}

static void handle_escape(EditorState* es, int ch)
{
	(void) ch;
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	print_screen(es);
	move_cursor_to_tab(t);
	es->mode = &normal_mode;
}

bool terminal_create(EditorState* es)
{
	for (int i = 0; i < NUM_CHARS; i++)
	{
		execute_char[i] = &handle_default;
	}
	execute_char[ESCAPE_KEYCODE] = &handle_escape;
	execute_char[ENTER_KEYCODE1] = &handle_enter;
	execute_char[BACKSPACE_KEYCODE2] = &handle_backspace;

	terminal = malloc(sizeof(Tab));
	if (terminal == NULL)
	{
		return false;
	}
	terminal->fname = NULL;
	terminal->undos = NULL;
	terminal->lines = ll_create();
	if (terminal->lines == NULL)
	{
		free(terminal);
		return false;
	}
	Line* l = malloc(sizeof(Line));
	if (l == NULL)
	{
		ll_free(terminal->lines);
		free(terminal);
		return false;
	}
	l->gb = gb_create(NULL, -1);
	if (l->gb == NULL)
	{
		ll_free(terminal->lines);
		free(terminal);
		line_free(l);
	}
	ll_insert(terminal->lines, l, 0);

	set_tab_to_fill_screen(terminal);
	terminal->x = 0;
	terminal->y = 0;
	terminal->pt = NULL;
	terminal->xpos = 0;
	terminal->ypos = terminal->height - TERMINAL_HEIGHT;
	terminal->height = TERMINAL_HEIGHT;
	terminal->left_column_index = 0;
	terminal->top_line_index = 0;

	slave_pid = forkpty(&master_fd, NULL, NULL, NULL);
	if (slave_pid == 0)
	{
		execlp("bash", "bash", NULL);
	}
	else if (slave_pid == -1)
	{
		tab_free(terminal);
		return false;
	}

	if (pthread_create(&listener, NULL, &listener_func, es))
	{
		kill(slave_pid, SIGKILL);
		waitpid(slave_pid, NULL, 0);
		close(master_fd);
		tab_free(terminal);
		return false;
	}

	return true;
}

void terminal_free(EditorState* es)
{
	sem_wait(&es->sem);
	pthread_cancel(listener);
	pthread_join(listener, NULL);
	sem_post(&es->sem);
	if (listener_buf != NULL)
	{
		free(listener_buf);
	}
	kill(slave_pid, SIGKILL);
	waitpid(slave_pid, NULL, 0);
	close(master_fd);
	tab_free(terminal);
}

void terminal_mode(EditorState* es, int ch)
{
	if (es == NULL)
	{
		return;
	}
	(*execute_char[ch])(es, ch);
}

void* listener_func(void* v)
{
	EditorState* es = (EditorState*) v;
	while (!(es->flags & TERMINATE_FLAG))
	{
		listener_buf = malloc(sizeof(char) * LINE_SIZE);
		if (listener_buf == NULL)
		{
			log_error("malloc failed in forkpty thread\n");
			continue;
		}
		int bytes_read = read(master_fd, listener_buf, LINE_SIZE - 1);
		listener_buf[bytes_read] = '\0';
		if (bytes_read > 0)
		{
			sem_wait(&es->sem);
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
					ll_insert(terminal->lines, l, terminal->lines->size - 1);
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
			ll_insert(terminal->lines, l, terminal->lines->size - 1);
			terminal->y++;

			free(listener_buf);
			listener_buf = NULL;

			check_bottom_update(terminal);
			if (es->mode == &terminal_mode)
			{
				move_cursor_to_tab(terminal);
			}
			print_tab(terminal);
			refresh();
			sem_post(&es->sem);
		}
		else
		{
			sem_wait(&es->sem);
			free(listener_buf);
			listener_buf = NULL;
			sem_post(&es->sem);
		}
	}

	return NULL;
}
