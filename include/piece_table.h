#ifndef PIECE_TABLE_H
#define PIECE_TABLE_H

#include "tree.h"

/*
 * !! WHEN USING PIECE STRUCT AS A FINDER TO LOOKUP PIECES IN PieceTable->pieces !!
 * to search by line:
 * set lines_contained to line_index, chars_contained to -1, and use piece_compare_lines. global index of the first character of the returned piece will be in chars_contained, global line_index of the first characters will be in lines_inside
 * to search by character:
 * set chars_contained to char_index + 1, lines_inside to -1, and use piece_compare. global index of the first character of the returned piece will be in len
*/

typedef struct Piece
{
	char** text;
	int start_index;
	int len;
	int lines_contained; // includes '\n's in this piece and all subpieces (left and right)
	int lines_inside; // number of '\n's between start_index and len
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
int piece_compare(Tree* t, void* elt);
int piece_compare_lines(Tree* t, void* elt);
void update_info(Tree* t);

void print_piece(void* v);

#endif
