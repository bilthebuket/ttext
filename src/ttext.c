#include <stdio.h>
#include <ncurses.h>
#include "LL.h"
#include "global.h"

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

	active_tab = (Tab*) get(tabs, 0);
	print_tab(active_tab);
	mode = &normal_mode;

	initscr();
	noecho();
	cbreak();

	while (1)
	{
		(*mode)(getch());
		refresh();
	}

	endwin();
}
