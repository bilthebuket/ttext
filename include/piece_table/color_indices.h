#ifndef PIECE_TABLE_COLOR_INDICES_H
#define PIECE_TABLE_COLOR_INDICES_H

#include <stdbool.h>
#include "piece_table/piece_table.h"
#include "tree.h"

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

void ci_init_arrays(void);
void ci_uninit_arrays(void);

int pt_get_color(PieceTable* pt, int index);
void pt_update_color_indices(PieceTable* pt, int index);
void ci_update_info(Tree* t);
ColorIndex* ci_create(int color, int len, int chars_contained);

int ci_compare(Tree* t, void* elt);
int ci_finder_compare_characters(Tree* t, void* elt);

void ci_prepare(PieceTable* pt, int index);
void ci_handle_insert(PieceTable* pt);
void ci_handle_rm(PieceTable* pt);
bool ci_execute(PieceTable* pt, int* first_line_updated, int* last_line_updated);

int iterate_to_start_of_update_chunk(PieceTable* pt, int start_index);
int iterate_to_end_of_update_chunk(PieceTable* pt, int end_index);

bool is_control_word(char* s);

void merge_color_indices_on_boundary(PieceTable* pt, int start_index, int end_index);

char ci_get_len(void* v);
char ci_get_color(void* v);

bool is_control_char(char c);

#endif
