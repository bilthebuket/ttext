#ifndef UNDO_H
#define UNDO_H

#include "global.h"

typedef struct UndoInfo
{
	int index;
	int num_deleted;
	int num_added;
} UndoInfo;

void undo_insert(EditorState* es, int index);
void undo_prepare_for_execute(EditorState* es);
void undo_execute(EditorState* es);
void undo_handle_insert(EditorState* es);
void undo_handle_delete(EditorState* es);

#endif
