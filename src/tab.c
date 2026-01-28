#include <stdio.h>
#include <stdlib.h>
#include "tab.h"
#include "global.h"
#include "line.h"
#include "io_tools.h"

Tab* make_tab(char* fname)
{
	Tab* r = malloc(sizeof(Tab));
	if (r == NULL)
	{
		log_error("malloc failed in make_tab\n");
		return NULL;
	}
	r->fname = fname;
	r->z_index_changes_saved = CHANGES_SAVED;
	r->x = 0;
	r->y = 0;
	r->height = height - 1;
	r->width = width;
	r->xpos = 0;
	r->ypos = 0;
	r->top_line_index = 0;
	r->left_column_index = 0;
	r->lines = make_list();
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
		char* buf = malloc(sizeof(char) * LINE_SIZE);
		if (buf == NULL)
		{
			log_error("malloc failed in make_tab\n");
			free(r);
			return NULL;
		}
		buf[0] = '\0';

		Line* l = malloc(sizeof(Line));
		if (l == NULL)
		{
			log_error("malloc failed in make_tab\n");
			free(r);
			free(buf);
			return NULL;
		}
		l->text = buf;
		l->color_indices = NULL;
		add(r->lines, l, 0);
		return r;
	}

	while (1)
	{
		char* buf = malloc(sizeof(char) * LINE_SIZE);
		if (buf == NULL)
		{
			log_error("malloc failed in make_tab\n");
			free_tab(r);
			fclose(f);
			return NULL;
		}
		// setting the last character in the buffer to a random character that is not the null character so we can check
		// after fgets() to see if the buffer was filled completely, in which case the line is too long for the editor we 
		// terminate
		buf[LINE_SIZE - 1] = 'a';
		buf[0] = '\0';

		if (!fgets(buf, LINE_SIZE, f))
		{
			free(buf);
			break;
		}
		if (buf[LINE_SIZE - 1] == '\0')
		{
			log_error("attempted to load file with a line exceeding the max line size\n");
			free_tab(r);
			fclose(f);
			free(buf);
			return NULL;
		}

		int len;
		for (len = 0; buf[len] != '\0'; len++) {}
		if (buf[len - 1] == '\n')
		{
			buf[len - 1] = '\0';
			len--;
		}

		convert_tabs_to_spaces(buf);

		Line* l = malloc(sizeof(Line));
		if (l == NULL)
		{
			log_error("malloc failed in make_tab\n");
			free_tab(r);
			fclose(f);
			free(buf);
			return NULL;
		}

		l->text = buf;
		l->color_indices = NULL;
		update_color_indices(l);
		add(r->lines, l, r->lines->size);
	}

	fclose(f);
	return r;
}

void free_tab(Tab* t)
{
	if (t != NULL)
	{
		if (t->fname != NULL)
		{
			free(t->fname);
		}
		void* elt;
		while ((elt = rm(t->lines, 0)) != NULL)
		{
			Line* l = (Line*) elt;
			if (l->text != NULL)
			{
				free(l->text);
			}
			if (l->color_indices != NULL)
			{
				free_list(l->color_indices);
			}
			free(l);
		}
		free_list(t->lines);
		free(t);
	}
}
