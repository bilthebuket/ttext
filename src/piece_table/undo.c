#include <stdlib.h>
#include "piece_table/undo.h"
#include "piece_table/piece_table.h"

// creates a new set of undos
void pt_undo_insert(PieceTable* pt)
{
	Undo** elt = malloc(sizeof(Undo*));
	elt[0] = NULL;
	ll_insert(pt->undos, elt, 0);
}

// updates the set of undos at the top of the undo stack
void pt_undo_update(PieceTable* pt, Undo* to_add)
{
	Undo** latest_undos = (Undo**) ll_get_elt(pt->undos, 0);

	// if the new undo is updating the same piece as the first undo in the undos that is at the top of the stack,
	// we can just combine them. this will be utilized heavily when computing undos from insert mode,
	// as that undo will be a bunch of adjacent inserts/deletes, thus they will be combineable
	if (latest_undos[0] != NULL)
	{
		if (latest_undos[0]->operation == UNDO_UPDATE && to_add->operation == UNDO_UPDATE)
		{
			UndoUpdate* existing = (UndoUpdate*) latest_undos[0]->stuff_we_need;
			UndoUpdate* new = (UndoUpdate*) to_add->stuff_we_need;

			if (existing->p == new->p)
			{
				free(new);
				free(to_add);
				return;
			}
		}
	}

	int len = 0;
	for (; latest_undos[len] != NULL; len++) {}
	len++;

	Undo** new_latest_undos = malloc(sizeof(Undo*) * (len + 1));
	for (int i = 0; i < len; i++)
	{
		new_latest_undos[i + 1] = latest_undos[i];
	}
	new_latest_undos[0] = to_add;
	
	free(latest_undos);
	ll_rm(pt->undos, 0);
	ll_insert(pt->undos, new_latest_undos, 0);
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
	}
	free(u);
}

void pt_undo_execute(PieceTable* pt)
{
	Undo** undos = (Undo**) ll_rm(pt->undos, 0);
	if (undos == NULL)
	{
		return;
	}

	for (int i = 0; undos[i] != NULL; i++)
	{
		Undo* to_execute = undos[i];

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

				Tree* to_update = tree_helper(pt->pieces, &f, piece_finder_compare_characters);
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
