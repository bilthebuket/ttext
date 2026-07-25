#ifndef SIGNATURE_H
#define SIGNATURE_H

#include <stdio.h>
#include "hash_map.h"
#include "piece_table/piece_table.h"

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

void update_signatures(HashMap* signatures, PieceTable* pt, const char* file_name, int index);
HashMap* initialize_signatures(void);

Signature* signature_create(char* signature, char* file_name);
bool signature_equals(void* v1, void* v2);
void signature_free(void* v);

bool function_name_equals(void* v1, void* v2);

// returns signature's function name if in a signature, or NULL if it is not
char* in_signature_huh(HashMap* signatures, PieceTable* pt, int index, int* store_start_index);

// file_name should be a single file name, or NULL to remove all instances of these function names
void remove_signatures(HashMap* signatures, LinkedList* function_names, char* file_name);
void remove_signature(HashMap* signatures, char* function_name, char* file_name);

void update_signatures_on_boundary(HashMap* signatures, PieceTable* pt, char* file_name, int start_index, int end_index);

void su_prepare(HashMap* signatures, PieceTable* pt, SignatureUpdate* su, char* file_name, int index);
void su_handle_insertion(SignatureUpdate* su, int index);
void su_handle_deletion(SignatureUpdate* su, int index);
void su_execute(HashMap* signatures, PieceTable* pt, SignatureUpdate* su, char* file_name);

void print_all_signatures(HashMap* signatures, FILE* f);

#endif
