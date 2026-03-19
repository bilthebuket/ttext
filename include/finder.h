#ifndef FINDER_H
#define FINDER_H

#include "piece_table.h"
#include "tree.h"

typedef struct FinderFinder
{
	int global_char_index;
	int contained;
} FinderFinder;

typedef struct FinderNode
{
	int contained;
	int len;
} FinderNode;

typedef struct Finder
{
	char* looking_for;
	Tree* indices_found;
} Finder;

Finder* finder_create(PieceTable* pt, char* looking_for);
void finder_update(Finder* f, PieceTable* pt);
void finder_free(Finder* f);
void find_next(Tab* t, Finder* f);

int finder_node_compare(Tree* t, void* v);
int finder_finder_compare(Tree* t, void* v);

#endif
