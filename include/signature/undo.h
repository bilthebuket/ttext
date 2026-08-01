#ifndef SIGNATURE_UNDO_H
#define SIGNATURE_UNDO_H

#define SIGNATURE_UNDO_CREATE 1
#define SIGNATURE_UNDO_REMOVE 2

#include "signature/signature.h"

// s and function_name are the same pointers that are/were used in the hashmap
// this means that if you have a SIGNATURE_UNDO_REMOVE, you do not need to free s and function_name when its executed
// because calling signature_remove will free those same chunks of memory
typedef struct SignatureUndo
{
	Signature* signature;
	char* function_name;
	int operation;
} SignatureUndo;

void signature_undo_insert(Signatures* signatures, SignatureUndo* undo);
void signature_undo_execute(Signatures* signatures);
void signature_undo_new(Signatures* signatures);
SignatureUndo* signature_undo_create(Signature* s, char* function_name, int operation);
void signature_undo_free(SignatureUndo* su);

#endif
