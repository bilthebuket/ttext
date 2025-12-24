#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pty.h>
#include <unistd.h>
#include <pthread.h>
#include "LL.h"
#include "global.h"
#include "io_tools.h"
#include "normal_mode.h"
#include "terminal_mode.h"

int main(int argc, char* argv[])
{
	initscr();
	noecho();
	cbreak();
	getmaxyx(stdscr, height, width);

	terminal = malloc(sizeof(Tab));
	terminal->lines = make_list();
	terminal->width = width;
	terminal->height = 5;
	terminal->x = 1;
	terminal->y = terminal->height - 1;
	terminal->xpos = 0;
	terminal->ypos = height - terminal->height;
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
		Tab* t = make_tab(argv[i]);
		if (!t)
		{
			print_message("There is at least one file with at least one line that is too long");
		}
		else
		{
			add(tabs, t, tabs->size);
		}
	}

	active_tab = (Tab*) get_elt(tabs, 0);
	mode = &normal_mode;

	print_tab(active_tab);
	refresh();

	while (1)
	{
		(*mode)(getch());
		refresh();
	}

	endwin();
}
