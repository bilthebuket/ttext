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

void print_all_signatures(HashMap* signatures, FILE* f);

// index is the index in pt of the character that was just added
void handle_character_addition(HashMap* Signatures, PieceTable* pt, char* file_name, int index);
// index is the index where the removed character used to be (now points to the character to the right of the removed character)
void handle_character_removal(HashMap* signatures, PieceTable* pt, int index);

#endif
