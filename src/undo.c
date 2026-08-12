#include <stdlib.h>
#include "piece_table/color_indices.h"
#include "piece_table/piece_table.h"
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

	PieceIterator pi;
	*start_index = ui->index - ui->num_deleted;

	if (!pt_iterator_init(pt, &pi, *start_index))
	{
		return false;
	}

	char c = pt_iterate_backwards(&pi);
	while (c != '\n' && c != '\0')
	{
		c = pt_iterate_backwards(&pi);
		*start_index = *start_index - 1;
	}
	*start_index = *start_index + 1;

	*end_index = ui->index + ui->num_added - ui->num_deleted;
	if (!pt_iterator_init(pt, &pi, ui->index + ui->num_added - ui->num_deleted))
	{
		return false;
	}

	c = pt_iterate(&pi);
	while (c != '\n' && c != '\0')
	{
		c = pt_iterate(&pi);
		*end_index = *end_index + 1;
	}
	*end_index = *end_index - 1;

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
		merge_color_indices_on_boundary(pt, start_index, end_index);

		ColorIndexFinder f;
		f.contained = start_index + 1;
		f.global_char_index = -1;
		Tree* t = tree_helper(pt->color_indices, &f, &ci_finder_compare_characters);
		if (t != NULL && t->elt != NULL)
		{
			ColorIndex* ci = (ColorIndex*) t->elt;
			ci->len += ui->num_deleted;
			ci->len -= ui->num_added;
			tree_recursive_update_to_root(t, &ci_update_info);
		}
	}
}

void undo_prepare_for_execute(EditorState* es)
{
	if (es == NULL || es->active_tab == NULL || es->active_tab->undos == NULL)
	{
		return;
	}

	UndoInfo* ui = ll_get_elt(es->active_tab->undos, 0);
	if (es->active_tab->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		signature_undo_prepare_for_execute(es->signatures, es->active_tab->pt, es->active_tab->fname, ui);
	}
	color_indices_prepare_for_execute(es->active_tab->pt, ui);
}

static bool get_post_bounds(PieceTable* pt, UndoInfo* ui, int* start_index, int* end_index)
{
	if (pt == NULL || ui == NULL || start_index == NULL || end_index == NULL)
	{
		return false;
	}

	PieceIterator pi;
	*start_index = ui->index - ui->num_deleted;

	if (!pt_iterator_init(pt, &pi, *start_index))
	{
		return false;
	}

	char c = pt_iterate_backwards(&pi);
	while (c != '\n' && c != '\0')
	{
		c = pt_iterate_backwards(&pi);
		*start_index = *start_index - 1;
	}
	*start_index = *start_index + 1;

	*end_index = ui->index;
	if (!pt_iterator_init(pt, &pi, ui->index))
	{
		return false;
	}

	c = pt_iterate(&pi);
	while (c != '\n' && c != '\0')
	{
		c = pt_iterate(&pi);
		*end_index = *end_index + 1;
	}
	*end_index = *end_index - 1;

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
		f.global_char_index = start_index;
		update_until_no_update_occurs(pt, f, 0);
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

	if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		signature_undo_execute(es->signatures, es->active_tab->pt, es->active_tab->fname, ui);
	}
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

	Tab* t = es->active_tab;
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
