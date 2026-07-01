#ifndef SIGNATURE_H
#define SIGNATURE_H

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

#endif
