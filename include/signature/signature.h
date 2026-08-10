#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <stdio.h>
#include "hash_map.h"
#include "piece_table/piece_table.h"
#include "linked_list.h"

#define IN_SIGNATURE_HUH_NUM_RETURN_VALS 2
#define IN_SIGNATURE_HUH_FUNCTION_NAME 0
#define IN_SIGNATURE_HUH_SIGNATURE 1

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

void update_signatures(HashMap* signatures, LinkedList* undos, PieceTable* pt, const char* file_name, int index, bool undo);
HashMap* initialize_signatures(void);

Signature* signature_create(char* signature, char* file_name);

// loose equality; if one of the signatures has a NULL field that isn't NULL for the other, it can still evaluate to true
bool signature_equals_loose(void* v1, void* v2);
bool signature_equals_strict(void* v1, void* v2);
void signature_free(void* v);

bool function_name_equals(void* v1, void* v2);

// returns signature's function name if in a signature, or NULL if it is not
char** in_signature_huh(HashMap* signatures, PieceTable* pt, int index, int* store_start_index);

// file_name should be a single file name, or NULL to remove all instances of this function name
void remove_signature(HashMap* signatures, LinkedList* undos, char* function_name, char* signature, char* file_name, bool undo);

void update_signatures_on_boundary(HashMap* signatures, LinkedList* undos, PieceTable* pt, char* file_name, int start_index, int end_index, bool undo);

void su_prepare(HashMap* signatures, LinkedList* undos, PieceTable* pt, SignatureUpdate* su, char* file_name, int index);
void su_handle_insertion(HashMap* signatures, LinkedList* undos, PieceTable* pt, char* file_name, SignatureUpdate* su, int index);
void su_handle_deletion(HashMap* signatures, LinkedList* undos, PieceTable* pt, char* file_name, SignatureUpdate* su, int index);
void su_execute(HashMap* signatures, LinkedList* undos, PieceTable* pt, SignatureUpdate* su, char* file_name);

void print_all_signatures(HashMap* signatures, FILE* f);

#endif
