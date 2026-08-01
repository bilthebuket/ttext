#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <stdio.h>
#include "hash_map.h"
#include "piece_table/piece_table.h"

#define IN_SIGNATURE_HUH_NUM_RETURN_VALS 2
#define IN_SIGNATURE_HUH_FUNCTION_NAME 0
#define IN_SIGNATURE_HUH_SIGNATURE 1

typedef struct Signatures
{
	HashMap* signatures;
	// doubly linked list of SignatureUndos (each element is a linked list of undos to be executed all at once)
	LinkedList* undos;
} Signatures;

typedef struct Signature
{
	char* signature;
	// the name of the file where this function is defined
	char* file_name;
} Signature;

typedef struct SignatureUpdate
{
	int start_index;
	int end_index;
} SignatureUpdate;

void update_signatures(Signatures* signatures, PieceTable* pt, const char* file_name, int index);
Signatures* initialize_signatures(void);
void signatures_free(Signatures* signatures);

Signature* signature_create(char* signature, char* file_name);

// loose equality; if one of the signatures has a NULL field that isn't NULL for the other, it can still evaluate to true
bool signature_equals_loose(void* v1, void* v2);
bool signature_equals_strict(void* v1, void* v2);
void signature_free(void* v);

bool function_name_equals(void* v1, void* v2);

// returns signature's function name if in a signature, or NULL if it is not
char** in_signature_huh(Signatures* signatures, PieceTable* pt, int index, int* store_start_index);

// file_name should be a single file name, or NULL to remove all instances of this function name
void remove_signature(Signatures* signatures, char* function_name, char* signature, char* file_name);

void update_signatures_on_boundary(Signatures* signatures, PieceTable* pt, char* file_name, int start_index, int end_index);

void su_prepare(Signatures* signatures, PieceTable* pt, SignatureUpdate* su, char* file_name, int index);
void su_handle_insertion(Signatures* signatures, PieceTable* pt, char* file_name, SignatureUpdate* su, int index);
void su_handle_deletion(Signatures* signatures, PieceTable* pt, char* file_name, SignatureUpdate* su, int index);
void su_execute(Signatures* signatures, PieceTable* pt, SignatureUpdate* su, char* file_name);

void print_all_signatures(Signatures* signatures, FILE* f);

#endif
