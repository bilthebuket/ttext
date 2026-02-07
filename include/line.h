#ifndef LINE_H
#define LINE_H

#include "LL.h"

typedef struct GapBuffer
{
	// this is the index of the character in text that is immediatly to the left of the buffer
	// (the one that the cursor is currently on)
	int gap_index;
	int gap_size;

	// size of text
	int text_size;
	// number of bytes in text that are actually storing text
	// includes the null character
	int num_chars;
	char* text;
} GapBuffer;

typedef struct ColorIndex
{
	int index;
	int color;
} ColorIndex;

typedef struct Line
{
	GapBuffer* gb;
	LL* color_indices;
} Line;

int gb_goto(GapBuffer* gb, int index);
int gb_goleft(GapBuffer* gb);
int gb_goright(GapBuffer* gb);
int gb_put(GapBuffer* gb, char c);
int gb_rm(GapBuffer* gb);
int gb_get(GapBuffer* gb, int index);
void free_gb(GapBuffer* gb);

void update_color_indices(Line* line);
void free_line(Line* line);

#endif
