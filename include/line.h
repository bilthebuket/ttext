#ifndef LINE_H
#define LINE_H

#include "LL.h"

typedef struct ColorIndex
{
	int index;
	int color;
} ColorIndex;

typedef struct Line
{
	char* text;
	LL* color_indices;
} Line;

void update_color_indices(Line* line);
void free_line(Line* line);

#endif
