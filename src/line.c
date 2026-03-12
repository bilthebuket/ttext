#include <stdlib.h>
#include <string.h>
#include "io_tools.h"
#include "line.h"
#include "global.h"

void free_line(Line* line)
{
	if (line == NULL)
	{
		return;
	}
	gb_free(line->gb);
	free(line);
}

GapBuffer* gb_create(char* text, int text_size)
{
	GapBuffer* gb = malloc(sizeof(GapBuffer));
	if (gb == NULL)
	{
		return NULL;
	}

	gb->gap_index = 0;
	gb->gap_size = GB_SIZE;

	if (text != NULL)
	{
		int len = 0;
		for (; text[len] != '\0'; len++) {}
		if (text_size < 0)
		{
			gb->gap_size = 0;
			gb->text = text;
			gb->num_chars = len + 1;
			gb->text_size = text_size;
			return gb;
		}
		else if (len + GB_SIZE >= text_size)
		{
			char* new_text = malloc(sizeof(char) * (text_size + LINE_SIZE));
			if (new_text == NULL)
			{
				free(gb);
				return NULL;
			}
			for (int i = 0; text[i] != '\0'; i++)
			{
				new_text[i + GB_SIZE] = text[i];
			}
			new_text[len + GB_SIZE] = '\0';
			free(text);
			text = new_text;
			gb->text_size = text_size + LINE_SIZE;
		}
		else
		{
			for (int i = len; i > 0; i--)
			{
				text[i + GB_SIZE] = text[i];
			}
			gb->text_size = text_size;
		}
		gb->text = text;
		gb->num_chars = len + 1;
	}
	else
	{
		gb->text = malloc(sizeof(char) * LINE_SIZE);
		if (gb->text == NULL)
		{
			free(gb);
			return NULL;
		}
		gb->num_chars = 1;
		gb->text_size = LINE_SIZE;
		gb->text[0] = '\0';
	}
	return gb;
}

int gb_goto(GapBuffer* gb, int index)
{
	if (gb == NULL)
	{
		return -1;
	}
	if (index > gb->gap_index)
	{
		if (index >= gb->num_chars)
		{
			return -1;
		}
		for (; gb->gap_index != index; gb->gap_index++)
		{
			gb->text[gb->gap_index + 1] = gb->text[gb->gap_index + gb->gap_size + 1];
		}
	}
	else if (index < gb->gap_index)
	{
		if (index < 0)
		{
			return -1;
		}
		for (; gb->gap_index != index; gb->gap_index--)
		{
			gb->text[gb->gap_index + gb->gap_size] = gb->text[gb->gap_index];
		}
	}
	else
	{
		return gb->text[index];
	}

	return gb->text[index];
}

int gb_goleft(GapBuffer* gb)
{
	return gb_goto(gb, gb->gap_index - 1);
}

int gb_goright(GapBuffer* gb)
{
	return gb_goto(gb, gb->gap_index + 1);
}

int gb_put(GapBuffer* gb, char c)
{
	if (gb == NULL)
	{
		return -1;
	}
	if (gb->gap_size == 0)
	{
		// plus one because we're planning to add a character on top of rebuffering
		if (gb->num_chars + GB_SIZE + 1 > gb->text_size)
		{
			char* new_text = malloc(sizeof(char) * (gb->text_size + LINE_SIZE));
			if (new_text == NULL)
			{
				return -1;
			}
			for (int i = 0; i <= gb->gap_index; i++)
			{
				new_text[i] = gb->text[i];
			}
			for (int i = gb->gap_index + 1; i < gb->text_size; i++)
			{
				new_text[i + GB_SIZE] = gb->text[i];
			}
			free(gb->text);
			gb->text = new_text;
			gb->text_size += LINE_SIZE;
			gb->gap_size = GB_SIZE;
		}
		else
		{
			for (int i = gb->num_chars - 1; i > gb->gap_index; i--)
			{
				gb->text[i + GB_SIZE] = gb->text[i];
			}
			gb->gap_size = GB_SIZE;
		}
	}

	gb->text[gb->gap_index + 1] = gb->text[gb->gap_index];
	gb->text[gb->gap_index] = c;
	gb->gap_index++;
	gb->num_chars++;
	gb->gap_size--;
	return gb->gap_index;
}

int gb_rm(GapBuffer* gb)
{
	if (gb == NULL)
	{
		return -1;
	}
	if (gb->num_chars < 2)
	{
		return -1;
	}
	if (gb->text[gb->gap_index] != '\0')
	{
		char c = gb->text[gb->gap_index];
		gb->text[gb->gap_index] = gb->text[gb->gap_index + gb->gap_size + 1];
		gb->gap_size++;
		gb->num_chars--;
		return c;
	}
	return -1;
}

int gb_get(GapBuffer* gb, int index)
{
	if (gb == NULL)
	{
		return -1;
	}
	if (index < 0 || index >= gb->num_chars)
	{
		return -1;
	}
	if (index > gb->gap_index)
	{
		return gb->text[index + gb->gap_size];
	}
	return gb->text[index];
}

void gb_free(GapBuffer* gb)
{
	if (gb != NULL)
	{
		if (gb->text != NULL)
		{
			free(gb->text);
		}
		free(gb);
	}
}

int gb_strcmp(GapBuffer* gb, int start_index, int end_index, const char* const str)
{
	if (gb == NULL || str == NULL || start_index > end_index)
	{
		return -2;
	}
	bool same = true;
	for (int i = start_index; i < end_index; i++)
	{
		if (gb_get(gb, i) != str[i - start_index])
		{
			same = false;
		}
		if (str[i - start_index] == '\0')
		{
			return 1;
		}
	}
	if (str[end_index - start_index] != '\0')
	{
		return -1;
	}
	if (same)
	{
		return 0;
	}
	return 2;
}

int gb_atoi(GapBuffer* gb, int start_index, int end_index)
{
	int index = gb->gap_index;
	gb_goto(gb, end_index);
	char c = gb->text[end_index];
	gb->text[end_index] = '\0';
	char* ptr = &gb->text[start_index];
	int r = atoi(ptr);
	gb->text[end_index] = c;
	gb_goto(gb, index);
	return r;
}
