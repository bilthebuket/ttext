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

typedef struct CI
{
	int index;
	int color;
} CI;

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
// if text is not NULL and text_size is less than 0, gb struct will be created but will have no gap (used for lines from forkpty(), as those do not need to be modified)
GapBuffer* gb_create(char* text, int text_size);
void gb_free(GapBuffer* gb);

// start_index is inclusive, end_index is not
// returns 0 if same, 1 if gb is longer than str, -1 if str is longer than gb, -2 if there was invalid arg(s), and 2 if they are same length but different strings
int gb_strcmp(GapBuffer* gb, int start_index, int end_index, const char* const str);
int gb_atoi(GapBuffer* gb, int start_index, int end_index);

void update_color_indices(Line* line);
void free_line(Line* line);

#endif
