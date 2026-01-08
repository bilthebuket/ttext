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
	LL* ColorIndices
} Line;

void update_color_indices(Line* line);

#endif
