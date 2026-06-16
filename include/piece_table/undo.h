#ifndef PIECE_TABLE_UNDO_H
#define PIECE_TABLE_UNDO_H

#define UNDO_CREATE 1
#define UNDO_UPDATE 2
#define UNDO_RM 3
#define UNDO_CI_CREATE 4
#define UNDO_CI_UPDATE 5
#define UNDO_CI_RM 6

#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"

typedef struct Undo
{
	// points to a struct that contains the data required to execute whatever type of undo this is
	// makes more sense when you look at pt_undo_execute
	void* stuff_we_need;
	int operation;
} Undo;

typedef struct Undos
{
	Undo** pieces;
	Undo** color_indices;
} Undos;

typedef struct UndoUpdate
{
	Piece* p;
	int index;
	int start_index;
	int len;
	int lines_inside;
} UndoUpdate;

typedef struct UndoUpdateColorIndex
{
	ColorIndex* ci;
	int index;
	int chars_contained;
	int len;
	int color;
} UndoUpdateColorIndex;

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

/*
 * naming scheme:
 * undo: where are dealing with Undos
 * update/rm/create: when pt_undo_execute is called on this undo, it will update, remove, or create a Piece/ColorIndex
 * color_index: we are dealing with ColorIndex's instead of Piece's (originally ColorIndex's weren't handled at all by undos which is why piece isn't specified in the first three function names)
 * create: we are creating an Undo struct
*/

Undo* undo_update_create(Piece* p, int index, int start_index, int len, int lines_inside);
Undo* undo_rm_create(int index);
Undo* undo_create_create(Piece* p);

Undo* undo_update_color_index_create(ColorIndex* ci, int index, int chars_contained, int len, int color);
Undo* undo_rm_color_index_create(int index);
Undo* undo_create_color_index_create(ColorIndex* ci);

#endif
