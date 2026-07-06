#include <stdio.h>
#include <stdlib.h>
#include <ncurses.h>
#include <pty.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include "linked_list.h"
#include "global.h"
#include "io_tools.h"
#include "normal_mode.h"
#include "terminal_mode.h"
#include "insert_mode.h"
#include "line.h"
#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"

int main(int argc, char* argv[])
{
	error_log = fopen("errors.log", "a");
	if (error_log == NULL)
	{
		fprintf(stderr, "could not open error log");
		return 1;
	}

	screen_create();
	print_message("initializing...");

	EditorState es;
	if (es_init(&es, argc, argv))
	{
		screen_free();
		return 1;
	}

	sem_wait(&es.sem);
	print_screen(&es);
	refresh();
	sem_post(&es.sem);

	print_message("normal mode");

	while (!(es.flags & TERMINATE_FLAG))
	{
		char c = getch();
		sem_wait(&es.sem);
		(*es.mode)(&es, c);
		refresh();
		sem_post(&es.sem);
	}

	if (error_log != NULL)
	{
		fclose(error_log);
	}

	screen_free();
	es_uninit(&es);
}
