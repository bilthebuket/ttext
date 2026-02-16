#ifndef PIECE_H
#define PIECE_H

#include "LL.h"

typedef struct Piece
{
	char* text;
	LL* sub_pieces;
	int text_len; // number of charcters in text excluding null char
	int text_size;
	int index; // index in the main buffer's array where this piece sits
	int chars_removed;
} Piece;

Piece* make_piece(int index);
void merge_oldest(Piece* p);
void handle_input(Piece* p, char c);
void free_piece(Piece* p);

#endif
