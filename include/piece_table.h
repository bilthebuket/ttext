#ifndef PIECE_TABLE_H
#define PIECE_TABLE_H

#include "tree.h"
#include <stdbool.h>

#define NUM_PRIME_NUMBERS 8
#define MAX_HASH_VALUE 2000

typedef struct ColorIndex
{
	int chars_contained;
	int len;
	int color;
} ColorIndex;

// works the same as PieceFinder
typedef struct ColorIndexFinder
{
	int contained;
	int global_char_index;
} ColorIndexFinder;

typedef struct Piece
{
	char** text;
	int start_index;
	int len;
	int lines_contained; // includes '\n's in this piece and all subpieces (left and right)
	int lines_inside; // number of '\n's between start_index and len
	int chars_contained; // same as lines_contained except for all characters
} Piece;

typedef struct PieceIterator
{
	Tree* node;
	int index;
} PieceIterator;

/*
 * How to use:
 * to search by line, set contained to line_index, use finder_compare_lines
 * to search by character, set contained to char_index + 1, use finder_compare_characters
*/

typedef struct PieceFinder
{
	int contained;

	// make sure to set global_char_index to -1 before handing it to tree_helper/tree_get
	int global_char_index; // global char index of first character in piece
	int global_line_index; // global line index of first character in piece
} PieceFinder;

typedef struct PieceTable
{
	char* original;
	char* append;
	Tree* pieces;
	Tree* color_indices;
	int append_size;
	int append_len;
} PieceTable;

void piece_table_init_arrays(void);

void pt_insert(PieceTable* pt, char c, int index);
void pt_rm(PieceTable* pt, int index);
char pt_get(PieceTable* pt, int index);
bool pt_iterator_init(PieceTable* pt, PieceIterator* pi, int index);
char pt_iterate(PieceIterator* pi);
PieceTable* pt_create(char* buf, int len);
void pt_free(PieceTable* pt);

// gets the index of the first character in the line in the piece table of index line_index
int pt_get_line_index(PieceTable* pt, int line_index);

int pt_get_color(PieceTable* pt, int index);
void pt_update_color_indices(PieceTable* pt, int index);
void color_index_update_info(Tree* t);
ColorIndex* make_color_index(int color, int len, int chars_contained);

int color_index_compare(Tree* t, void* elt);
int color_index_finder_compare_characters(Tree* t, void* elt);

Piece* make_piece(char** text, int start_index, int len, int chars_contained);
void free_piece(void* v);

int piece_compare(Tree* t, void* elt);
int piece_finder_compare_lines(Tree* t, void* elt);
int piece_finder_compare_characters(Tree* t, void* elt);

int piece_compare_lines(Tree* t, void* elt);
void piece_update_info(Tree* t);

void print_piece(void* v);
void print_color_index(void* v);

#endif
