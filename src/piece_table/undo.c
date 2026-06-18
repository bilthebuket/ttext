#include <stdlib.h>
#include "piece_table/undo.h"
#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"

// creates a new set of undos
void pt_undo_insert(PieceTable* pt)
{
	Undos* elt = malloc(sizeof(Undos));
	if (elt == NULL)
	{
		return;
	}

	elt->pieces = malloc(sizeof(Undo*));
	if (elt->pieces == NULL)
	{
		free(elt);
		return;
	}
	elt->color_indices = malloc(sizeof(Undo*));
	if (elt->color_indices == NULL)
	{
		free(elt->pieces);
		free(elt);
		return;
	}

	elt->pieces[0] = NULL;
	elt->color_indices[0] = NULL;
	ll_insert(pt->undos, elt, 0);
}

// updates the set of undos at the top of the undo stack
void pt_undo_update(PieceTable* pt, Undo* to_add)
{
	if (pt == NULL || to_add == NULL)
	{
		return;
	}

	Undos* latest_undos = (Undos*) ll_get_elt(pt->undos, 0);
	if (latest_undos == NULL)
	{
		pt_undo_insert(pt);
		latest_undos = (Undos*) ll_get_elt(pt->undos, 0);

		if (latest_undos == NULL)
		{
			if (!(to_add->operation == UNDO_CREATE || to_add->operation == UNDO_CI_CREATE))
			{
				free(to_add->stuff_we_need);
			}
			free(to_add);
			return;
		}
	}

	if (to_add->operation == UNDO_CREATE || to_add->operation == UNDO_RM || to_add->operation == UNDO_UPDATE)
	{
		Undo** piece_undos = latest_undos->pieces;
		if (piece_undos[0] != NULL && to_add->operation == UNDO_UPDATE && piece_undos[0]->operation == UNDO_UPDATE)
		{
			UndoUpdate* existing = (UndoUpdate*) piece_undos[0]->stuff_we_need;
			UndoUpdate* new = (UndoUpdate*) to_add->stuff_we_need;

			if (existing->p == new->p)
			{
				existing->index = new->index;
				undo_free(to_add);
				return;
			}
		}

		int len = 0;
		for (; piece_undos[len] != NULL; len++) {}
		len++;

		Undo** new_piece_undos = malloc(sizeof(Undo*) * (len + 1));
		for (int i = 0; i < len; i++)
		{
			new_piece_undos[i + 1] = piece_undos[i];
		}
		new_piece_undos[0] = to_add;
		
		free(piece_undos);
		latest_undos->pieces = new_piece_undos;
	}
	else
	{
		Undo** ci_undos = latest_undos->color_indices;
		if (ci_undos[0] != NULL && to_add->operation == UNDO_CI_UPDATE && ci_undos[0]->operation == UNDO_CI_UPDATE)
		{
			UndoUpdateColorIndex* existing = (UndoUpdateColorIndex*) ci_undos[0]->stuff_we_need;
			UndoUpdateColorIndex* new = (UndoUpdateColorIndex*) to_add->stuff_we_need;

			if (existing->ci == new->ci)
			{
				existing->index = new->index;
				undo_free(to_add);
				return;
			}
		}

		int len = 0;
		for (; ci_undos[len] != NULL; len++) {}
		len++;

		Undo** new_ci_undos = malloc(sizeof(Undo*) * (len + 1));
		for (int i = 0; i < len; i++)
		{
			new_ci_undos[i + 1] = ci_undos[i];
		}
		new_ci_undos[0] = to_add;
		
		free(ci_undos);
		latest_undos->color_indices = new_ci_undos;
	}
}

void undo_free(Undo* u)
{
	if (u == NULL)
	{
		return;
	}

	switch (u->operation)
	{
		default:
		{
			free(u->stuff_we_need);
			break;
		}
		case UNDO_CREATE:
		{
			piece_free((Piece*) u->stuff_we_need);
			break;
		}
		case UNDO_CI_CREATE:
		{
			free((ColorIndex*) u->stuff_we_need);
			break;
		}
	}
	free(u);
}

void pt_undo_execute(PieceTable* pt)
{
	Undos* undos = (Undos*) ll_rm(pt->undos, 0);
	if (undos == NULL)
	{
		return;
	}

	for (int i = 0; undos->pieces[i] != NULL; i++)
	{
		Undo* to_execute = undos->pieces[i];

		switch (to_execute->operation)
		{
			case UNDO_CREATE:
			{
				Piece* p = (Piece*) to_execute->stuff_we_need;
				pt->pieces = tree_add_elt(pt->pieces, p, &piece_compare, &piece_update_info);
				break;
			}

			case UNDO_UPDATE:
			{
				UndoUpdate* u = (UndoUpdate*) to_execute->stuff_we_need;

				PieceFinder f;
				f.contained = u->index + 1;
				f.global_char_index = -1;

				Tree* to_update = tree_helper(pt->pieces, &f, &piece_finder_compare_characters);
				if (to_update == NULL || to_update->elt == NULL)
				{
					return;
				}
				Piece* piece_to_update = (Piece*) to_update->elt;

				piece_to_update->start_index = u->start_index;
				piece_to_update->len = u->len;
				piece_to_update->lines_inside = u->lines_inside;
				tree_recursive_update_to_root(to_update, &piece_update_info);
				free(u);
				break;
			}

			case UNDO_RM:
			{
				PieceFinder f;
				f.contained = *((int*) to_execute->stuff_we_need);
				f.global_char_index = -1;
				pt->pieces = tree_rm(pt->pieces, &f, &piece_finder_compare_characters, &piece_free, &piece_update_info);
				free(to_execute->stuff_we_need);
				break;
			}
		}

		free(to_execute);
	}

	for (int i = 0; undos->color_indices[i] != NULL; i++)
	{
		Undo* to_execute = undos->color_indices[i];

		switch(to_execute->operation)
		{
			case UNDO_CI_CREATE:
			{
				ColorIndex* ci = (ColorIndex*) to_execute->stuff_we_need;
				pt->color_indices = tree_add_elt(pt->color_indices, ci, &ci_compare, &ci_update_info);
				break;
			}

			case UNDO_CI_UPDATE:
			{
				UndoUpdateColorIndex* u = (UndoUpdateColorIndex*) to_execute->stuff_we_need;

				ColorIndexFinder f;
				f.contained = u->index + 1;
				f.global_char_index = -1;

				Tree* to_update = tree_helper(pt->color_indices, &f, &ci_finder_compare_characters);
				if (to_update == NULL || to_update->elt == NULL)
				{
					return;
				}
				ColorIndex* ci_to_update = (ColorIndex*) to_update->elt;

				ci_to_update->chars_contained = u->chars_contained;
				ci_to_update->len = u->len;
				ci_to_update->color = u->color;
				tree_recursive_update_to_root(to_update, &ci_update_info);
				free(u);
				break;
			}

			case UNDO_CI_RM:
			{
				ColorIndexFinder f;
				f.contained = *((int*) to_execute->stuff_we_need);
				f.global_char_index = -1;
				pt->color_indices = tree_rm(pt->color_indices, &f, &ci_finder_compare_characters, &free, &ci_update_info);
				free(to_execute->stuff_we_need);
				break;
			}
		}

		free(to_execute);
	}
	free(undos->pieces);
	free(undos->color_indices);
	free(undos);
}

Undo* undo_update_create(Piece* p, int index, int start_index, int len, int lines_inside)
{
	Undo* r = malloc(sizeof(Undo));
	if (r != NULL)
	{
		UndoUpdate* u = malloc(sizeof(UndoUpdate));
		if (u != NULL)
		{
			u->p = p;
			u->start_index = start_index;
			u->len = len;
			u->lines_inside = lines_inside;
			u->index = index;

			r->stuff_we_need = u;
			r->operation = UNDO_UPDATE;
		}
		else
		{
			free(r);
			return NULL;
		}
	}

	return r;
}

Undo* undo_rm_create(int index)
{
	Undo* r = malloc(sizeof(Undo));
	if (r != NULL)
	{
		int* indexptr = malloc(sizeof(int));
		if (indexptr == NULL)
		{
			free(r);
			return NULL;
		}
		*indexptr = index;
		r->stuff_we_need = indexptr;
		r->operation = UNDO_RM;
	}
	return r;
}

Undo* undo_create_create(Piece* p)
{
	if (p == NULL)
	{
		return NULL;
	}

	Undo* r = malloc(sizeof(Undo));
	if (r != NULL)
	{
		r->stuff_we_need = p;
		r->operation = UNDO_CREATE;
	}
	return r;
}

Undo* undo_update_color_index_create(ColorIndex* ci, int index, int chars_contained, int len, int color)
{
	Undo* r = malloc(sizeof(Undo));
	if (r != NULL)
	{
		UndoUpdateColorIndex* u = malloc(sizeof(UndoUpdate));
		if (u != NULL)
		{
			u->ci = ci;
			u->index = index;
			u->chars_contained = chars_contained;
			u->len = len;
			u->color = color;

			r->stuff_we_need = u;
			r->operation = UNDO_CI_UPDATE;
		}
		else
		{
			free(r);
			return NULL;
		}
	}
	return r;
}

Undo* undo_rm_color_index_create(int index)
{
	Undo* r = malloc(sizeof(Undo));
	if (r != NULL)
	{
		int* indexptr = malloc(sizeof(int));
		if (indexptr == NULL)
		{
			free(r);
			return NULL;
		}
		*indexptr = index;
		r->stuff_we_need = indexptr;
		r->operation = UNDO_CI_RM;
	}
	return r;
}

Undo* undo_create_color_index_create(ColorIndex* ci)
{
	if (ci == NULL)
	{
		return NULL;
	}

	Undo* r = malloc(sizeof(Undo));
	if (r != NULL)
	{
		r->stuff_we_need = ci;
		r->operation = UNDO_CI_CREATE;
	}
	return r;
}
