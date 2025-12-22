#include <stdio.h>
#include <ncurses.h>
#include <pty.h>
#include <unistd.h>
#include "LL.h"
#include "global.h"
#include "io_tools.h"
#include "normal_mode.h"

int main(int argc, char* argv[])
{
	int master_fd;
	int pid = forkpty(&master_fd, NULL, NULL, NULL);
	if (pid == 0)
	{
		execlp("bash", "bash", NULL);
	}

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

	initscr();
	noecho();
	cbreak();
	getmaxyx(stdscr, height, width);

	print_tab(active_tab);
	refresh();

	while (1)
	{
		(*mode)(getch());
		refresh();
	}

	endwin();
}
