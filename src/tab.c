#include <stdio.h>
#include <stdlib.h>
#include "tab.h"
#include "global.h"
#include "line.h"
#include "io_tools.h"

Tab* make_tab(char* fname)
{
	Tab* r = malloc(sizeof(Tab));
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

	FILE* f;
	if (fname != NULL)
	{
		f = fopen(fname, "r");
	}
	else
	{
		f = NULL;
	}

	if (f == NULL)
	{
		char* buf = malloc(sizeof(char) * LINE_SIZE);
		buf[0] = '\0';
		Line* l = malloc(sizeof(Line));
		l->text = buf;
		l->color_indices = NULL;
		add(r->lines, l, 0);
		return r;
	}

	while (1)
	{
		char* buf = malloc(sizeof(char) * LINE_SIZE);
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
			free(buf);
			free_list(r->lines);
			free(fname);
			fclose(f);
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
	if (t->fname != NULL)
	{
		free(t->fname);
	}
	while (t->lines->size > 0)
	{
		Line* l = (Line*) rm(t->lines, 0);
		free(l->text);
		if (l->color_indices != NULL)
		{
			free_list(l->color_indices);
		}
		free(l);
	}
	free_list(t->lines);
	free(t);
}
