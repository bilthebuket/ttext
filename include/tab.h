#ifndef TAB_H
#define TAB_H

#include "LL.h"

typedef struct Tab
{
	LL* lines;
	char* fname;

	// cursor position
	int x;
	int y;

	int height;
	int width;

	// position of top left corner
	int xpos;
	int ypos;

	int top_line_index;
	int left_column_index;
} Tab;

Tab* make_tab(char* fname);

#endif
