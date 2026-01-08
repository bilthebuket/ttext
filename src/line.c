#include "line.h"
#include "global.h"

void update_color_indices(Line* line)
{
	int i = 0;
	while (1)
	{
		ColorIndex* ci = malloc(sizeof(ColorIndex));
		ci->index = i;
		for (; line->text[i] != '\0' && line->text[i] != ' ' && line->text[i] != '('; i++) {}
		if (line->text[i] == '(')
		{
			ci->color = YELLOW_TEXT;
		}
		else
		{
			char* ptr = &line->text[ci->index];
			line->text[i] = 
