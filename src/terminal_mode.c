#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include "terminal_mode.h"
#include "global.h"
#include "io_tools.h"

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
			
		}
		else
		{
			int i = 0;
			for (; line[i] != '\0'; i++) {}
			write(master_fd, line, i + 1);
			char* input_line = malloc(sizeof(char) * LINE_SIZE);
			input_line[0] = '\0';
			add(terminal->lines, input_line, terminal->lines->size);
			terminal->x = 0;
			terminal->y++;
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
	}
}

void* listener_func(void*)
{
	while (1)
	{
		char* buf = malloc(sizeof(char) * LINE_SIZE);
		read(master_fd, buf, LINE_SIZE);
		sem_wait(&sem);
		add(terminal->lines, buf, terminal->lines->size - 1);
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
}
