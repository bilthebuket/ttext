#include <stdlib.h>
#include <stddef.h>
#include "io_tools.h"
#include "normal_mode.h"
#include "insert_mode.h"
#include "terminal_mode.h"
#include "piece_table/color_indices.h"
#include "global.h"

FILE* error_log = NULL;

int es_init(EditorState* es, int argc, char* argv[])
{
	es->flags = 0;
	es->finder = NULL;
	sem_init(&(es->sem), 0, 1);
	ci_init_arrays();


	insert_mode_create();
	normal_mode_create();
	if (!terminal_create(es))
	{
		sem_destroy(&(es->sem));
		fprintf(stderr, "Could not initalize terminal\n");
		return 1;
	}

	es->tabs = ll_create();
	if (es->tabs == NULL)
	{
		sem_destroy(&(es->sem));
		terminal_free(es);
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
			t->tab_num_flags |= es->tabs->size;
			ll_insert(es->tabs, t, es->tabs->size);
		}
	}

	if (argc == 1)
	{
		es->active_tab = tab_create(NULL);
		ll_insert(es->tabs, es->active_tab, 0);
		es->active_tab_index = 0;
	}
	else
	{
		es->active_tab = (Tab*) ll_get_elt(es->tabs, argc - 2);
		es->active_tab_index = argc - 2;
	}
	es->mode = &normal_mode;

	return 0;
}

void es_uninit(EditorState* es)
{
	terminal_free(es);
	sem_destroy(&es->sem);
	while (es->tabs->size > 0)
	{
		Tab* t = ll_rm(es->tabs, 0);
		tab_free(t);
	}
	ll_free(es->tabs);
	finder_free(es->finder);
}
