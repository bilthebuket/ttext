#include <stdlib.h>
#include "signature/undo.h"
#include "signature/signature.h"
#include "global.h"

void signature_undo_insert(Signatures* signatures, SignatureUndo* undo)
{
	if (signatures == NULL || undo == NULL)
	{
		return;
	}

	LinkedList* lst = (LinkedList*) ll_get_elt(signatures->undos, 0);
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
			ll_insert(signatures->undos, undo, 0);
		}
	}
}

void signature_undo_execute(Signatures* signatures)
{
	if (signatures == NULL)
	{
		return;
	}

	LinkedList* lst = ll_rm(signatures->undos, 0);
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
				hm_insert(signatures->signatures, undo->function_name, undo->signature, &hash_function);
				break;
			}

			case SIGNATURE_UNDO_REMOVE:
			{
				remove_signature(signatures, undo->function_name, undo->signature->signature, undo->signature->file_name);
				signature_undo_free(undo);
				break;
			}
		}

		free(undo);
	}

	ll_free(lst);
}

void signature_undo_new(Signatures* signatures)
{
	if (signatures == NULL || signatures->undos == NULL)
	{
		return;
	}

	LinkedList* lst = ll_create();
	if (lst != NULL)
	{
		ll_insert(signatures->undos, lst, 0);
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
