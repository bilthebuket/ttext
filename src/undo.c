#include <stdlib.h>
#include "piece_table/color_indices.h"
#include "piece_table/piece_table.h"
#include "undo.h"
#include "global.h"

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

	if (t->undos->size > MAX_NUM_UNDOS)
	{
		UndoInfo* to_remove = ll_rm(t->undos, t->undos->size - 1);
		if (to_remove != NULL)
		{
			free(to_remove);
		}
	}

	if (t->redos != NULL)
	{
		while (t->redos->size > 0)
		{
			UndoInfo* ui = ll_rm(t->redos, 0);
			if (ui != NULL)
			{
				free(ui);
			}
		}
	}
}

static bool get_pre_bounds(PieceTable* pt, UndoInfo* ui, int* start_index, int* end_index)
{
	if (ui == NULL || start_index == NULL || end_index == NULL)
	{
		return false;
	}

	*start_index = iterate_to_start_of_update_chunk(pt, ui->index - ui->num_deleted - 1);
	if (*start_index < 0)
	{
		return false;
	}

	*end_index = iterate_to_end_of_update_chunk(pt, ui->index + ui->num_added - ui->num_deleted);
	if (*end_index < 0)
	{
		return false;
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

static void color_indices_prepare_for_execute(PieceTable* pt, UndoInfo* ui)
{
	if (pt == NULL || ui == NULL)
	{
		return;
	}

	if (pt->pieces == NULL && pt->color_indices == NULL)
	{
		ColorIndex* ci = ci_create(CYAN_TEXT, ui->num_deleted, ui->num_deleted);
		if (ci != NULL)
		{
			pt->color_indices = tree_create(ci);
			if (pt->color_indices == NULL)
			{
				free(ci);
			}
		}
	}
	else
	{
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
				if (ci->len + ui->num_deleted - ui->num_added > 0)
				{
					ci->len += ui->num_deleted;
					ci->len -= ui->num_added;
					tree_recursive_update_to_root(t, &ci_update_info);
				}
				else
				{
					f.contained = start_index + 1;
					f.global_char_index = -1;
					pt->color_indices = tree_rm(pt->color_indices, &f, &ci_finder_compare_characters, &free, &ci_update_info);
				}
			}
		}
	}
}

static void prepare_helper(EditorState* es, UndoInfo* ui)
{
	if (es == NULL || es->active_tab == NULL || es->active_tab->undos == NULL)
	{
		return;
	}

	if (es->active_tab->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		signature_undo_prepare_for_execute(es->signatures, es->active_tab->pt, es->active_tab->fname, ui);
	}
	color_indices_prepare_for_execute(es->active_tab->pt, ui);
}

void undo_prepare_for_execute(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	UndoInfo* ui = ll_get_elt(t->undos, 0);
	prepare_helper(es, ui);
}

void redo_prepare_for_execute(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	UndoInfo* ui = ll_get_elt(t->redos, 0);
	prepare_helper(es, ui);
}

static bool get_post_bounds(PieceTable* pt, UndoInfo* ui, int* start_index, int* end_index)
{
	if (ui == NULL || start_index == NULL || end_index == NULL)
	{
		return false;
	}

	*start_index = iterate_to_start_of_update_chunk(pt, ui->index - ui->num_deleted);
	if (*start_index < 0)
	{
		return false;
	}


	*end_index = iterate_to_end_of_update_chunk(pt, ui->index);
	if (*end_index < 0)
	{
		return false;
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

	if (pt->pieces == NULL)
	{
		tree_free(pt->color_indices, &free);
		pt->color_indices = NULL;
		return;
	}

	int start_index;
	int end_index;
	if (get_post_bounds(pt, ui, &start_index, &end_index))
	{
		pt_update_color_indices(pt, start_index);
	}
}

static void execute_helper(EditorState* es, UndoInfo* ui)
{
	if (es == NULL || es->active_tab == NULL)
	{
		return;
	}

	Tab* t = es->active_tab;

	if (t->tab_num_flags & PARSE_FOR_SIGNATURES)
	{
		signature_undo_execute(es->signatures, es->active_tab->pt, es->active_tab->fname, ui);
	}
	color_indices_undo_execute(es->active_tab->pt, ui);
}

void undo_execute(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	UndoInfo* ui = ll_rm(t->undos, 0);
	execute_helper(es, ui);
	ll_insert(t->redos, ui, 0);
}

void redo_execute(EditorState* es)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return;
	}

	UndoInfo* ui = ll_rm(t->redos, 0);
	execute_helper(es, ui);
	ll_insert(t->undos, ui, 0);
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

void undo_handle_multiple_rm(EditorState* es, int num_deleted)
{
	if (es == NULL || es->active_tab == NULL)
	{
		return;
	}

	Tab* t = es->active_tab;
	UndoInfo* ui = ll_get_elt(t->undos, 0);

	if (ui == NULL)
	{
		return;
	}

	if (ui->num_added > 0)
	{
		if (ui->num_added > num_deleted)
		{
			ui->num_added -= num_deleted;
			num_deleted = 0;
		}
		else
		{
			num_deleted -= ui->num_added;
			ui->num_added = 0;
		}
	}

	ui->num_deleted += num_deleted;
}
