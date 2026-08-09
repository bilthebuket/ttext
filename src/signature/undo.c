#include <stdlib.h>
#include "signature/undo.h"
#include "signature/signature.h"
#include "global.h"

void signature_undo_insert(HashMap* signatures, LinkedList* undos, SignatureUndo* undo)
{
	if (signatures == NULL || undos == NULL || undo == NULL)
	{
		if (undo->operation == SIGNATURE_UNDO_CREATE)
		{
			signature_undo_free(undo);
		}
		return;
	}

	LinkedList* lst = (LinkedList*) ll_get_elt(undos, 0);
	if (lst == NULL)
	{
		if (undo->operation == SIGNATURE_UNDO_CREATE)
		{
			signature_undo_free(undo);
		}
	}
	else
	{
		bool unnecessary = false;
		for (int i = 0; i < lst->size; i++)
		{
			SignatureUndo* existing_undo = (SignatureUndo*) ll_get_elt(lst, i);
			if (existing_undo == NULL)
			{
				continue;
			}
			if ((existing_undo->operation == SIGNATURE_UNDO_REMOVE && undo->operation == SIGNATURE_UNDO_CREATE) || (existing_undo->operation == SIGNATURE_UNDO_CREATE && undo->operation == SIGNATURE_UNDO_REMOVE))
			{
				if (signature_equals_strict(existing_undo->signature, undo->signature) && function_name_equals(existing_undo->function_name, undo->function_name))
				{
					unnecessary = true;
					break;
				}
			}
		}

		if (unnecessary)
		{
			if (undo->operation == SIGNATURE_UNDO_CREATE)
			{
				signature_undo_free(undo);
			}
		}
		else
		{
			ll_insert(undos, undo, 0);
		}
	}
}

void signature_undo_execute(HashMap* signatures, LinkedList* undos)
{
	if (signatures == NULL || undos == NULL)
	{
		return;
	}

	LinkedList* lst = ll_rm(undos, 0);
	if (lst == NULL)
	{
		return;
	}

	while (lst->size > 0)
	{
		SignatureUndo* undo = ll_rm(lst, 0);

		if (undo == NULL)
		{
			continue;
		}

		switch (undo->operation)
		{
			case SIGNATURE_UNDO_CREATE:
			{
				hm_insert(signatures, undo->function_name, undo->signature, &hash_function);
				break;
			}

			case SIGNATURE_UNDO_REMOVE:
			{
				remove_signature(signatures, undos, undo->function_name, undo->signature->signature, undo->signature->file_name, false);
				signature_undo_free(undo);
				break;
			}
		}

		free(undo);
	}

	ll_free(lst);
}

void signature_undo_new(LinkedList* undos)
{
	if (undos == NULL)
	{
		return;
	}

	LinkedList* lst = ll_create();
	if (lst != NULL)
	{
		ll_insert(undos, lst, 0);
	}
}

SignatureUndo* signature_undo_create(Signature* s, char* function_name, int operation)
{
	if (s == NULL || function_name == NULL || (operation != SIGNATURE_UNDO_CREATE && operation != SIGNATURE_UNDO_REMOVE))
	{
		return NULL;
	}

	SignatureUndo* r = malloc(sizeof(SignatureUndo));
	if (r != NULL)
	{
		r->signature = s;
		r->function_name = function_name;
		r->operation = operation;
	}
	return r;
}

void signature_undo_free(SignatureUndo* su)
{
	if (su == NULL)
	{
		return;
	}

	signature_free(su->signature);
	if (su->function_name != NULL)
	{
		free(su->function_name);
	}
	free(su);
}

void signature_undos_free(LinkedList* undos)
{
	if (undos == NULL)
	{
		return;
	}

	while (undos->size > 0)
	{
		LinkedList* lst = (LinkedList*) ll_rm(undos, 0);
		if (lst == NULL)
		{
			continue;
		}

		while (lst->size > 0)
		{
			SignatureUndo* su = (SignatureUndo*) ll_rm(lst, 0);
			signature_undo_free(su);
		}

		ll_free(lst);
	}

	ll_free(undos);
}
