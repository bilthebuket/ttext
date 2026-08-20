#ifndef PIECE_TABLE_H
#define PIECE_TABLE_H

#include "tree.h"
#include "linked_list.h"
#include <stdbool.h>

// when you are inserting a piece into pt->pieces, set piece->chars_contained to global index of the last character of the piece + 1
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

	// each element in this linked list is an Undo** which must be terminated with a null pointer
	LinkedList* undos;
	int append_size;
	int append_len;

	int ci_index;
	int ci_chars_added;
} PieceTable;

void pt_insert(PieceTable* pt, char c, int index);
void pt_rm(PieceTable* pt, int index);
char pt_get(PieceTable* pt, int index);
bool pt_iterator_init(PieceTable* pt, PieceIterator* pi, int index);
char pt_iterate(PieceIterator* pi);
char pt_iterate_backwards(PieceIterator* pi);
PieceTable* pt_create(char* buf, int len, bool do_color_indices);
void pt_free(PieceTable* pt);
char* pt_flatten_to_str(PieceTable* pt);

// gets the index of the first character in the line in the piece table of index line_index
int pt_get_line_index(PieceTable* pt, int line_index);

// gets the index of the line that a character resides in
int pt_get_line_index_inverse(PieceTable* pt, int char_index);

Piece* piece_create(char** text, int start_index, int len, int chars_contained);
void piece_free(void* v);

int piece_compare(Tree* t, void* elt);
int piece_finder_compare_lines(Tree* t, void* elt);
int piece_finder_compare_characters(Tree* t, void* elt);

int piece_compare_lines(Tree* t, void* elt);
void piece_update_info(Tree* t);

void print_piece(void* v);
void print_color_index(void* v);

void piece_iterator_copy(PieceIterator* to, PieceIterator* from);

#endif
