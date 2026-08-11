#include <stdlib.h>
#include "undo.h"

void undo_insert(EditorState* es, int index)
{
	if (es == NULL || es->active_tab == NULL)
	{
		return;
	}

	Tab* t = es->active_tab;
	if (t->undos == NULL)
	{
		t->undos = ll_create();
		if (t->undos == NULL)
		{
			return;
		}
	}

	UndoInfo* ui = malloc(sizeof(UndoInfo));
	if (ui == NULL)
	{
		return;
	}

	ui->index = index;
	ui->num_deleted = 0;
	ui->num_added = 0;

	ll_insert(t->undos, ui, 0);
}

void undo_execute(EditorState* es)
{
	if (es == NULL || es->active_tab == NULL)
	{
		return;
	}

	Tab* t = es->active_tab;

	UndoInfo* ui = ll_rm(t->undos, 0);
	if (ui == NULL)
	{
		return;
	}

	signature_undo_execute(es, ui);
	color_indices_undo_execute(es, ui);
}

void undo_handle_insert(EditorState* es)
{
	if (es == NULL || es->active_tab == NULL)
	{
		return;
	}

	Tab* t = es->active_tab;
	UndoInfo* ui = ll_get_elt(t->undos, 0);
	ui->num_added++;
}
void undo_handle_delete(EditorState* es)
{
	if (es == NULL || es->active_tab == NULL)
	{
		return;
	}

	Tab* r = es->active_tab;
	UndoInfo* ui = ll_get_elt(t->undos, 0);
	
	if (ui->num_added == 0)
	{
		ui->num_deleted++;
	}
	else
	{
		ui->num_added--;
	}
}
