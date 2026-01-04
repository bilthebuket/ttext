#ifndef TAB_H
#define TAB_H

#include <stdint.h>
#include "LL.h"

typedef struct Tab
{
	LL* lines;
	char* fname;
	uint64_t z_index_changes_saved; // bits 0-62 are the z index, bit 63 is the changes_saved flag

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
void free_tab(Tab* t);

#endif
