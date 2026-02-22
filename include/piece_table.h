#ifndef PIECE_TABLE_H
#define PIECE_TABLE_H

#include "tree.h"

typedef struct Piece
{
	char** text;
	int start_index;
	int len;
	int lines_contained; // includes '\n's in this piece and all subpieces to the left
	int chars_contained; // same as lines_contained except for all characters
} Piece;

typedef struct PieceTable
{
	char* original;
	char* append;
	Tree* pieces;
	int append_size;
	int append_len;
} PieceTable;

void pt_insert(PieceTable* pt, char c, int index);
void pt_rm(PieceTable* pt, int index);
char pt_get(PieceTable* pt, int index);
PieceTable* pt_create(char* buf, int len);
void pt_free(PieceTable* pt);

// gets the index of the first character in the line in the piece table of index line_index
int pt_get_line_index(PieceTable* pt, int line_index);

Piece* make_piece(char** text, int start_index, int len, int chars_contained);
void free_piece(void* v);
int piece_compare(void* p1, void* p2);
int piece_compare_lines(void* p1, void* p2);
void update_info(Tree* t);

#endif
