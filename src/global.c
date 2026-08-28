#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include "io_tools.h"
#include "signature.h"
#include "normal_mode/normal_mode.h"
#include "insert_mode.h"
#include "terminal_mode.h"
#include "piece_table/color_indices.h"
#include "global.h"

bool is_valid_name_character(char c)
{
	return (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_' || c == '*' || c == '[' || c == ']';
}

FILE* error_log = NULL;

int es_init(EditorState* es, int argc, char* argv[])
{
	if (es == NULL)
	{
		return 1;
	}

	es->flags = 0;
	es->finder = NULL;
	es->action_repeat = 0;
	es->dependent_action = '\0';
	sem_init(&(es->sem), 0, 1);
	ci_init_arrays();


	es->mode = &normal_mode;
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
	if (argv == NULL)
	{
		es->signatures = hm_create();
	}
	else
	{
		es->signatures = initialize_signatures();
	}
	if (es->signatures == NULL)
	{
		es_uninit(es);
		return 1;
	}

	return 0;
}

void es_uninit(EditorState* es)
{
	ci_uninit_arrays();
	terminal_free(es);
	sem_destroy(&es->sem);
	while (es->tabs->size > 0)
	{
		Tab* t = ll_rm(es->tabs, 0);
		tab_free(t);
	}
	ll_free(es->tabs);
	finder_free(es->finder);
	hm_free(es->signatures, &free, &signature_free);
}

// TODO: use bitflags intead of bool array
static int prime_numbers[NUM_PRIME_NUMBERS] = {67, 283, 31, 593, 379, 389, 821, 113};

int hash_function(void* v, int max_value)
{
	char* s = (char*) v;
	if (s == NULL)
	{
		return -1;
	}
	uint64_t val = 0;
	for (int i = 0; s[i] != '\0'; i++)
	{
		val += ((int) s[i]) * prime_numbers[i % NUM_PRIME_NUMBERS];
	}
	return (int) (val % max_value);
}
