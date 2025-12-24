#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "terminal_mode.h"
#include "global.h"
#include "io_tools.h"

void terminal_mode(int ch)
{
	switch (ch)
	{
		default:
		break;

		case ENTER_KEYCODE1:
		break;

		case BACKSPACE_KEYCODE2:
		break;
	}
}

void* listener_func(void*)
{
	while (1)
	{
		char* buf = malloc(sizeof(char) * LINE_SIZE);
		read(master_fd, buf, LINE_SIZE);
		add(terminal->lines, buf, terminal->lines->size);
		print_tab(terminal);
		refresh();
	}
}
