#include <stdio.h>
#include <stdlib.h>
#include "tab.h"
#include "global.h"

Tab* make_tab(char* fname)
{
	Tab* r = malloc(sizeof(Tab));
	r->fname = fname;
	r->changes_saved = true;
	r->x = 0;
	r->y = 0;
	r->height = height - 1;
	r->width = width;
	r->xpos = 0;
	r->ypos = 0;
	r->top_line_index = 0;
	r->left_column_index = 0;
	r->lines = make_list();

	FILE* f = fopen(fname, "r");
	if (f == NULL)
	{
		char* buf = malloc(sizeof(char) * LINE_SIZE);
		buf[0] = '\0';
		add(r->lines, buf, 0);
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

		int i;
		for (i = 0; buf[i] != '\0'; i++) {}
		if (buf[i - 1] == '\n')
		{
			buf[i - 1] = '\0';
		}
		add(r->lines, buf, r->lines->size);
	}

	fclose(f);
	return r;
}

void free_tab(Tab* t)
{
	free(t->fname);
	free_list(t->lines);
}
