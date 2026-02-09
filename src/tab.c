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
	r->height = height - 2;
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
		l->gb = gb_create(NULL, -1);
		l->color_indices = NULL;
		add(r->lines, l, 0);
		return r;
	}

	fseek(f, 0, SEEK_END);
	int size = ftell(f);
	rewind(f);
	char* buf = malloc(sizeof(char) * (size + 1));
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
	buf[size] = '\0';
	int i = 0;
	int prev = 0;
	while (1)
	{
		for (; buf[i] != '\n' && buf[i] != '\0'; i++) {}
		int len = (((i - prev) / LINE_SIZE) + 1) * LINE_SIZE;
		char* text = malloc(sizeof(char) * len);
		if (text == NULL)
		{
			log_error("malloc failed in make_tab\n");
			free_tab(r);
			fclose(f);
			free(buf);
			return NULL;
		}
		int j;
		for (j = prev; j < i; j++)
		{
			text[j - prev] = buf[j];
		}
		text[j - prev] = '\0';

		prev = i + 1;
		i++;

		GapBuffer* gb = gb_create(text, len);
		convert_tabs_to_spaces(gb);

		Line* l = malloc(sizeof(Line));
		if (l == NULL)
		{
			log_error("malloc failed in make_tab\n");
			free_tab(r);
			fclose(f);
			free(buf);
			return NULL;
		}

		l->gb = gb;
		l->color_indices = NULL;
		update_color_indices(l);
		add(r->lines, l, r->lines->size);

		if (buf[j] == '\0')
		{
			break;
		}
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
			free_line(l);
		}
		free_list(t->lines);
		free(t);
	}
}
