#ifndef TAB_H
#define TAB_H

#include "linked_list.h"
#include "piece_table/piece_table.h"

typedef struct Tab
{
	LinkedList* lines;
	PieceTable* pt;
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

	int saved_x_index;
	int tab_num_flags;
} Tab;

Tab* tab_create(char* fname);
void tab_free(Tab* t);

#endif
