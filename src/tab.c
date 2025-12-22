#include <stdlib.h>
#include "tab.h"

#define LINE_SIZE 2048

Tab* make_tab(char* fname)
{
	Tab* r = malloc(sizeof(Tab));
	r->fname = fname;
	r->x = 0;
	r->y = 0;
	r->lines = make_list();

	FILE* f = fopen(fname, "r");
	if (f == NULL)
	{
		return r;
	}

	while (1)
	{
		char* buf = malloc(sizeof(char) * LINE_SIZE);
		buf[LINE_SIZE - 1] = 0;
		buf[0] = '\0';
		fgets(buf, LINE_SIZE, f);
		if (buf[LINE_SIZE - 1] == '\0')
		{
			free(buf);
			free_list(r->lines);
			free(fname);
			return NULL;
		}
		add(r->lines, buf, r->lines->size);
	}

	return r;
}

void free_tab(Tab* t)
{
	free(t->fname);
	free_list(t->lines);
}
