#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pty.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "LL.h"
#include "global.h"
#include "io_tools.h"
#include "normal_mode.h"
#include "terminal_mode.h"

int main(int argc, char* argv[])
{
	sem_init(&sem, 0, 1);

	initscr();
	noecho();
	cbreak();
	getmaxyx(stdscr, height, width);

	terminal = malloc(sizeof(Tab));
	terminal->lines = make_list();
	char* input_line = malloc(sizeof(char) * LINE_SIZE);
	input_line[0] = '\0';
	add(terminal->lines, input_line, 0);

	terminal->width = width;
	terminal->height = 5;
	terminal->x = 0;
	terminal->y = 0;
	terminal->xpos = 0;
	terminal->ypos = height - terminal->height - 1;
	terminal->left_column_index = 0;
	terminal->top_line_index = 0;

	slave_pid = forkpty(&master_fd, NULL, NULL, NULL);
	if (slave_pid == 0)
	{
		execlp("bash", "bash", NULL);
	}

	pthread_t listener;
	pthread_create(&listener, NULL, &listener_func, NULL);

	tabs = make_list();
	for (int i = 1; i < argc; i++)
	{
		char* fname = malloc(sizeof(char) * FNAME_SIZE);
		int j;
		for (j = 0; argv[i][j] != '\0'; j++)
		{
			fname[j] = argv[i][j];
		}
		fname[j] = '\0';

		Tab* t = make_tab(fname);
		if (!t)
		{
			print_message("There is at least one file with at least one line that is too long");
		}
		else
		{
			add(tabs, t, tabs->size);
		}
	}

	if (argc == 1)
	{
		active_tab = make_tab(NULL);
		add(tabs, active_tab, 0);
		active_tab_index = 0;
	}
	else
	{
		active_tab = (Tab*) get_elt(tabs, 0);
		active_tab_index = 0;
	}
	mode = &normal_mode;

	sem_wait(&sem);
	print_tab(active_tab);
	refresh();
	sem_post(&sem);

	while (!terminate)
	{
		char c = getch();
		sem_wait(&sem);
		(*mode)(c);
		refresh();
		sem_post(&sem);
	}

	sem_wait(&sem);
	pthread_cancel(listener);
	pthread_join(listener, NULL);
	if (listener_buf != NULL)
	{
		free(listener_buf);
	}
	sem_post(&sem);
	endwin();
	sem_destroy(&sem);
	close(master_fd);
	free_list(terminal->lines);
	free(terminal);
	for (int i = 0; i < tabs->size; i++)
	{
		Tab* t = get_elt(tabs, i);
		free_list(t->lines);
		if (t->fname != NULL)
		{
			free(t->fname);
		}
	}
	free_list(tabs);
}
