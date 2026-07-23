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

void update_signatures(HashMap* signatures, PieceTable* pt, const char* file_name, int index);
HashMap* initialize_signatures(void);

Signature* signature_create(char* signature, char* file_name);
bool signature_equals(void* v1, void* v2);
void signature_free(void* v);

bool function_name_equals(void* v1, void* v2);

// checks if index is within a signature, if true, then store_function_name will have the function name for that signature
// if false store_function_name is undefined
bool in_signature_huh(HashMap* signatures, PieceTable* pt, int index);

// file_name should be a single file name, or NULL to remove all instances of these function names
void remove_signatures(HashMap* signatures, LinkedList* function_names, char* file_name);
void remove_signature(HashMap* signatures, char* function_name, char* file_name);

void print_all_signatures(HashMap* signatures, FILE* f);

#endif
