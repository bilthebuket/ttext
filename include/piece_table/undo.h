#ifndef PIECE_TABLE_UNDO_H
#define PIECE_TABLE_UNDO_H

#define UNDO_CREATE 1
#define UNDO_UPDATE 2
#define UNDO_RM 3

#include "piece_table/piece_table.h"

typedef struct Undo
{
	// points to a struct that contains the data required to execute whatever type of undo this is
	// makes more sense when you look at pt_undo_execute
	void* stuff_we_need;
	int operation;
} Undo;

typedef struct UndoUpdate
{
	Piece* p;
	int index;
	int start_index;
	int len;
	int lines_inside;
} UndoUpdate;

/*
 * basic idea: we have a stack of undos, every time the user presses 'u' the latest set of changes will be popped off the undo stack and executed (pt_undo_execute)
 * when we want to add a new undo to the set of undos on top of the undo stack, use pt_undo_update
 * when we want to create a new set of undos on top of the undo stack, use pt_undo_insert
 * pt_undo_free just frees an Undo struct
 * the reason its done this way is because some actions (noteably insert mode) will create multiple Undo structs that need to be executed together when the user presses 'u'
 * in general only pt_insert and pt_rm should be calling pt_undo_update, and then exterior functions (normal_mode, terminal_mode, etc.) will call pt_undo_insert and pt_undo_execute
*/
void pt_undo_insert(PieceTable* pt);
void pt_undo_execute(PieceTable* pt);
void undo_free(Undo* u);
void pt_undo_update(PieceTable* pt, Undo* to_add);

Undo* undo_update_create(Piece* p, int index, int start_index, int len, int lines_inside);
Undo* undo_rm_create(int index);
Undo* undo_create_create(Piece* p);

#endif
