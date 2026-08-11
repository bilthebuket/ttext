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

static bool get_pre_bounds(PieceTable* pt, UndoInfo* ui, int* start_index, int* end_index)
{
	if (pt == NULL || ui == NULL || start_index == NULL || end_index == NULL)
	{
		return false;
	}

	*start_index = pt_get_line_index(pt, ui->index - ui->num_deleted);

	*end_index = ui->index + ui->num_added - ui->num_deleted;
	PieceIterator pi;
	if (!pt_iterator_init(pt, &pi, ui->index + ui->num_added - ui->num_deleted))
	{
		return false;
	}

	char c = pt_iterate(&pi);
	while (c != '\n' && c != '\0')
	{
		c = pt_iterate(&pi);
		*end_index = *end_index + 1;
	}

	return true;
}

static void signature_undo_prepare_for_execute(HashMap* signatures, PieceTable* pt, char* file_name, UndoInfo* ui)
{
	if (signatures == NULL || pt == NULL || ui == NULL)
	{
		return;
	}

	int start_index;
	int end_index;
	if (get_pre_bounds(pt, ui, &start_index, &end_index))
	{
		remove_signatures_on_boundary(signatures, pt, file_name, start_index, end_index);
	}
}

/*
static void color_indices_prepare_for_execute(PieceTable* pt, UndoInfo* ui)
{
	if (pt == NULL || ui == NULL)
	{
		return;
	}

	int start_index;
	int end_index;
	if (get_pre_bounds(pt, ui, &start_index, &end_index))
	{
		merge_color_indices_on_boundary(pt, start_index, end_index)
	}
}
*/

void undo_prepare_for_execute(EditorState* es)
{
	if (es == NULL || es->active_tab == NULL || es->active_tab->undos == NULL)
	{
		return;
	}

	UndoInfo* ui = ll_get_elt(es->active_tab->undos, 0);
	signature_undo_prepare_for_execute(es->signatures, es->active_tab->pt, es->active_tab->fname, ui);
}

static bool get_post_bounds(PieceTable* pt, UndoInfo* ui, int* start_index, int* end_index)
{
	if (pt == NULL || ui == NULL || start_index == NULL || end_index == NULL)
	{
		return false;
	}

	*start_index = pt_get_line_index(pt, ui->index - ui->num_deleted);

	*end_index = ui->index;
	PieceIterator pi;
	if (!pt_iterator_init(pt, &pi, ui->index))
	{
		return false;
	}

	char c = pt_iterate(&pi);
	while (c != '\n' && c != '\0')
	{
		c = pt_iterate(&pi);
		*end_index = *end_index + 1;
	}

	return true;
}

static void signature_undo_execute(HashMap* signatures, PieceTable* pt, char* file_name, UndoInfo* ui)
{
	if (signatures == NULL || pt == NULL || ui == NULL)
	{
		return;
	}

	int start_index;
	int end_index;
	if (get_post_bounds(pt, ui, &start_index, &end_index))
	{
		update_signatures_on_boundary(signatures, pt, file_name, start_index, end_index);
	}
}

static void color_indices_undo_execute(PieceTable* pt, UndoInfo* ui)
{
	if (pt == NULL || ui == NULL)
	{
		return;
	}

	int start_index;
	int end_index;
	if (get_post_bounds(pt, ui, &start_index, &end_index))
	{
		ColorIndexFinder f;
		f.contained = start_index + 1;
		ColorIndex* ci = tree_get(pt->color_indices, &f, &ci_finder_compare_characters);
		if (ci != NULL)
		{
			update_until_no_update_occurs(pt, f, ci->len);
		}
	}
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

	signature_undo_execute(es->signatures, es->active_tab->pt, es->active_tab->fname, ui);
	color_indices_undo_execute(es->active_tab->pt, ui);
	free(ui);
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
