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
#include "line.h"
#include "piece_table.h"

int main(int argc, char* argv[])
{
	EditorState es;
	es.terminate = false;
	sem_init(&es.sem, 0, 1);
	screen_create();
	pt_init_arrays();


	if (!terminal_create(&es))
	{
		endwin();
		sem_destroy(&es.sem);
		fprintf(stderr, "Could not initalize terminal\n");
		return 1;
	}

	es.tabs = ll_create();
	if (es.tabs == NULL)
	{
		screen_free();
		sem_destroy(&es.sem);
		terminal_free(&es);
		fprintf(stderr, "ll_create() failed when creating list of es.tabs\n");
		return 1;
	}
	for (int i = 1; i < argc; i++)
	{
		char* fname = malloc(sizeof(char) * FNAME_SIZE);
		if (fname == NULL)
		{
			print_message("malloc failed when trying to create tab");
			continue;
		}
		int j;
		for (j = 0; j < FNAME_SIZE && argv[i][j] != '\0'; j++)
		{
			fname[j] = argv[i][j];
		}
		fname[j] = '\0';

		Tab* t = tab_create(fname);
		if (!t)
		{
			print_message("There is at least one file with at least one line that is too long");
		}
		else
		{
			t->tab_num_flags &= FLAG_BITS;
			t->tab_num_flags |= es.tabs->size;
			ll_insert(es.tabs, t, es.tabs->size);
		}
	}

	if (argc == 1)
	{
		es.active_tab = tab_create(NULL);
		ll_insert(es.tabs, es.active_tab, 0);
		es.active_tab_index = 0;
	}
	else
	{
		es.active_tab = (Tab*) ll_get_elt(es.tabs, argc - 2);
		es.active_tab_index = argc - 2;
	}
	es.mode = &normal_mode;

	sem_wait(&es.sem);
	print_screen(&es);
	refresh();
	sem_post(&es.sem);

	error_log = fopen("errors.log", "a");

	while (!es.terminate)
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

	terminal_free(&es);
	screen_free();
	sem_destroy(&es.sem);
	while (es.tabs->size > 0)
	{
		Tab* t = ll_rm(es.tabs, 0);
		tab_free(t);
	}
	ll_free(es.tabs);
}
