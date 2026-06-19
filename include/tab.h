#ifndef TAB_H
#define TAB_H

#include "linked_list.h"
#include "piece_table/piece_table.h"

#define BACKUP_EDIT_THRESHOLD 50

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
	int edits_since_last_backup;
} Tab;

Tab* tab_create(char* fname);
void tab_free(Tab* t);

// increments t->edits_since_last_backup and checks if it's time for a backup, creates backup if it is
void backup_increment_and_check(Tab* t);

#endif
