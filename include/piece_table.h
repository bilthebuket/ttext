#ifndef PIECE_TABLE_H
#define PIECE_TABLE_H

#include "tree.h"
#include "linked_list.h"
#include <stdbool.h>

#define NUM_PRIME_NUMBERS 8
#define MAX_HASH_VALUE 2000

#define UNDO_CREATE 1
#define UNDO_UPDATE 2
#define UNDO_DELETE 3

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
 * to search by line, set contained to line_index, use piece_finder_compare_lines
 * to search by character, set contained to char_index + 1, use piece_finder_compare_characters
 * the reason its not line_index + 1 is because the piece_finder_compare_lines function is dealing with the number
 * of newline characters, which will always be 1 less than the number of lines, whereas piece_finder_compare_characters uses
 * the number of characters, which is equal to the character index plus one
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
	LinkedList* undos;
	int append_size;
	int append_len;
} PieceTable;

typedef struct Undo
{
	// points to a struct that contains the data required to execute whatever type of undo this is
	// makes more sense when you look at pt_undo_execute
	void* stuff_we_need;
	int operation;
} Undo;

typedef struct UndoUpdate
{
	Tree* to_update;
	int start_index;
	int len;
	int lines_inside;
} UndoUpdate;

void pt_init_arrays(void);

void pt_insert(PieceTable* pt, char c, int index);
void pt_rm(PieceTable* pt, int index);
char pt_get(PieceTable* pt, int index);
bool pt_iterator_init(PieceTable* pt, PieceIterator* pi, int index);
char pt_iterate(PieceIterator* pi);
PieceTable* pt_create(char* buf, int len);
void pt_free(PieceTable* pt);

// gets the index of the first character in the line in the piece table of index line_index
int pt_get_line_index(PieceTable* pt, int line_index);

// gets the index of the line that a character resides in
int pt_get_line_index_inverse(PieceTable* pt, int char_index);

int pt_get_color(PieceTable* pt, int index);
void pt_update_color_indices(PieceTable* pt, int index);
void ci_update_info(Tree* t);
ColorIndex* ci_create(int color, int len, int chars_contained);

int ci_compare(Tree* t, void* elt);
int ci_finder_compare_characters(Tree* t, void* elt);

Piece* piece_create(char** text, int start_index, int len, int chars_contained);
void piece_free(void* v);

int piece_compare(Tree* t, void* elt);
int piece_finder_compare_lines(Tree* t, void* elt);
int piece_finder_compare_characters(Tree* t, void* elt);

int piece_compare_lines(Tree* t, void* elt);
void piece_update_info(Tree* t);

void print_piece(void* v);
void print_color_index(void* v);

void pt_undo_insert(PieceTable* pt, Undo* elt);
void pt_undo_execute(PieceTable* pt);

#endif
