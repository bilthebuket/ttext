#include <stdio.h>
#include <stdlib.h>
#include "tab.h"
#include "global.h"
#include "line.h"
#include "io_tools.h"

Tab* tab_create(char* fname)
{
	Tab* r = malloc(sizeof(Tab));
	if (r == NULL)
	{
		log_error("malloc failed in make_tab\n");
		return NULL;
	}
	set_tab_to_fill_screen(r);
	r->fname = fname;
	r->tab_num_flags = CHANGES_SAVED;
	r->x = 0;
	r->y = 0;
	r->xpos = 0;
	r->ypos = 0;
	r->top_line_index = 0;
	r->left_column_index = 0;
	r->lines = NULL;
	r->saved_x_index = 0;

	FILE* f;
	if (fname != NULL)
	{
		f = fopen(fname, "r");
		if (f == NULL)
		{
			log_error("unable to open file in make_tab\n");
		}
	}
	else
	{
		f = NULL;
	}

	if (f == NULL)
	{
		r->pt = pt_create(NULL, -1);
		return r;
	}

	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	rewind(f);
	char* buf = malloc(sizeof(char) * size);
	if (buf == NULL)
	{
		log_error("malloc failed in make_tab\n");
		free(buf);
		free(r);
		fclose(f);
		return NULL;
	}
	if (fread(buf, sizeof(char), size, f) <= 0)
	{
		log_error("fgets failed in make_tab\n");
		free(buf);
		fclose(f);
		free(r);
		return NULL;
	}

	if (buf[0] == '\0')
	{
		r->pt = pt_create(NULL, -1);
	}
	else
	{
		r->pt = pt_create(buf, size);
	}

	fclose(f);
	return r;
}

void tab_free(Tab* t)
{
	if (t != NULL)
	{
		if (t->fname != NULL)
		{
			free(t->fname);
		}
		if (t->lines != NULL)
		{
			void* elt;
			while ((elt = ll_rm(t->lines, 0)) != NULL)
			{
				Line* l = (Line*) elt;
				line_free(l);
			}
			ll_free(t->lines);
		}
		pt_free(t->pt);
		free(t);
	}
}
