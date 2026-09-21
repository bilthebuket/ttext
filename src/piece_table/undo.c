#include <stdlib.h>
#include "piece_table/undo.h"
#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"
#include "global.h"

// creates a new set of undos
void pt_undo_insert(PieceTable* pt)
{
	if (pt == NULL || pt->undos == NULL)
	{
		return;
	}

	LinkedList* elt = ll_create();
	if (elt != NULL)
	{
		ll_insert(pt->undos, elt, 0);
	}

	if (pt->undos->size > MAX_NUM_UNDOS)
	{
		LinkedList* to_remove = ll_rm(pt->undos, pt->undos->size - 1);
		ll_free_good(to_remove, &undo_free);
	}

	if (pt->redos != NULL)
	{
		while (pt->redos->size > 0)
		{
			LinkedList* redo = ll_rm(pt->redos, 0);
			ll_free_good(redo, &undo_free);
		}
	}
}

// updates the set of undos at the top of the undo stack
void pt_undo_update(PieceTable* pt, Undo* to_add)
{
	if (pt == NULL || to_add == NULL)
	{
		return;
	}

	LinkedList* latest_undos = ll_get_elt(pt->undos, 0);
	if (latest_undos == NULL)
	{
		pt_undo_insert(pt);
		latest_undos = ll_get_elt(pt->undos, 0);

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
		LinkedList* piece_undos = latest_undos;
		Undo* most_recent = ll_get_elt(piece_undos, 0);
		if (most_recent != NULL && to_add->operation == UNDO_UPDATE && most_recent->operation == UNDO_UPDATE)
		{
			UndoUpdate* existing = (UndoUpdate*) most_recent->stuff_we_need;
			UndoUpdate* new = (UndoUpdate*) to_add->stuff_we_need;

			if (existing->p == new->p)
			{
				existing->index = new->index;
				undo_free(to_add);
				return;
			}
		}

		ll_insert(piece_undos, to_add, 0);
	}
}

void undo_free(void* v)
{
	if (v == NULL)
	{
		return;
	}

	Undo* u = (Undo*) v;

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

LinkedList* execute_helper(PieceTable* pt, LinkedList* to_execute)
{
	LinkedList* r = ll_create();

	while (1)
	{
		Undo* undo = (Undo*) ll_rm(to_execute, 0);
		if (undo == NULL)
		{
			break;
		}

		switch (undo->operation)
		{
			case UNDO_CREATE:
			{
				Piece* p = (Piece*) undo->stuff_we_need;
				Undo* inverse = undo_rm_create(p->chars_contained);
				if (inverse != NULL)
				{
					ll_insert(r, inverse, 0);
				}
				pt->pieces = tree_insert(pt->pieces, p, &piece_compare, &piece_update_info);
				break;
			}

			case UNDO_UPDATE:
			{
				UndoUpdate* u = (UndoUpdate*) undo->stuff_we_need;

				PieceFinder f;
				f.contained = u->index + 1;
				f.global_char_index = -1;

				Tree* to_update = tree_helper(pt->pieces, &f, &piece_finder_compare_characters);
				if (to_update == NULL || to_update->elt == NULL)
				{
					break;
				}
				Piece* piece_to_update = (Piece*) to_update->elt;

				Undo* inverse = undo_update_create(piece_to_update, f.global_char_index, piece_to_update->start_index, piece_to_update->len, piece_to_update->lines_inside);
				if (inverse != NULL)
				{
					ll_insert(r, inverse, 0);
				}

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
				f.contained = *((int*) undo->stuff_we_need);
				f.global_char_index = -1;
				Piece* p = tree_get(pt->pieces, &f, &piece_finder_compare_characters);

				if (p != NULL)
				{
					p->chars_contained = f.global_char_index + p->len;
					Undo* inverse = undo_create_create(p);
					if (inverse != NULL)
					{
						ll_insert(r, inverse, 0);
					}
				}

				f.contained = *((int*) undo->stuff_we_need);
				f.global_char_index = -1;

				pt->pieces = tree_rm(pt->pieces, &f, &piece_finder_compare_characters, NULL, &piece_update_info);
				free(undo->stuff_we_need);
				break;
			}
		}

		free(undo);
	}

	ll_free(to_execute);
	return r;
}

void pt_undo_execute(PieceTable* pt)
{
	if (pt == NULL)
	{
		return;
	}

	LinkedList* undos = (LinkedList*) ll_rm(pt->undos, 0);
	if (undos == NULL)
	{
		return;
	}

	LinkedList* to_redo = execute_helper(pt, undos);
	ll_insert(pt->redos, to_redo, 0);
}

void pt_redo_execute(PieceTable* pt)
{
	if (pt == NULL)
	{
		return;
	}

	LinkedList* redos = (LinkedList*) ll_rm(pt->redos, 0);
	if (redos == NULL)
	{
		return;
	}

	LinkedList* to_undo = execute_helper(pt, redos);
	ll_insert(pt->undos, to_undo, 0);
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
