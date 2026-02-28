#include <stdlib.h>
#include <string.h>
#include "io_tools.h"
#include "line.h"
#include "global.h"

void update_color_indices(Line* line)
{
	if (line == NULL)
	{
		return;
	}
	if (line->gb == NULL)
	{
		if (line->color_indices != NULL)
		{
			free_list(line->color_indices);
		}
		line->color_indices = NULL;
		return;
	}
	if (line->color_indices != NULL)
	{
		free_list(line->color_indices);
	}
	line->color_indices = make_list();

	GapBuffer* gb = line->gb;

	// after every iteration we need to increment i
	// however, there are a lot of continue statements, so instead of putting i++ before every continue,
	// i have it at the top. this means for the first iteration we need to start at i = -1 so it gets incremented to 0
	int i = -1;
	while (1)
	{
		i++;
		CI* ci = malloc(sizeof(CI));
		if (ci == NULL)
		{
			log_error("malloc failed in update_color_indices\n");
			return;
		}
		add(line->color_indices, ci, line->color_indices->size);
		ci->index = i;
		ci->color = -1;
		for (; gb_get(gb, i) != '\0' && gb_get(gb, i) != ' ' && gb_get(gb, i) != '(' && gb_get(gb, i) != ')' && gb_get(gb, i) != '[' && gb_get(gb, i) != ']' && 
		gb_get(gb, i) != '{' && gb_get(gb, i) != '}' && gb_get(gb, i) != '\'' && gb_get(gb, i) != '"' && gb_get(gb, i) != ';' && gb_get(gb, i) != ':'; i++)
		{
			if (gb_get(gb, i) == '/')
			{
				if (gb_get(gb, i + 1) == '/')
				{
					break;
				}
			}
		}

		if (gb_get(gb, i) == '/')
		{
			ci->color = GREEN_TEXT;
			break;
		}
		if (gb_get(gb, i) == '(')
		{
			ci->color = YELLOW_TEXT;
			continue;
		}

		if (gb_get(gb, i) == '\'' || gb_get(gb, i) == '"')
		{
			// even if theres spaces inside the quotes we want everything highlighted red, so this skips i to outside the quotes

			if (i == ci->index)
			{
				ci->color = RED_TEXT;
			}
			else
			{
				CI* ci = malloc(sizeof(CI));
				if (ci == NULL)
				{
					log_error("malloc failed in update_color_indices\n");
					return;
				}
				ci->index = i;
				ci->color = RED_TEXT;
				add(line->color_indices, ci, line->color_indices->size);
			}
			char c = gb_get(gb, i);
			i++;
			for (; gb_get(gb, i) != c && gb_get(gb, i) != '\0'; i++) {}
			if (gb_get(gb, i) == '\0')
			{
				break;
			}
		}

		// this if statement makes it so all grouping symbols except quotes are yellow, but then we still need to find out what color to highlight
		// the text that precedes it
		if (gb_get(gb, i) != '\0' && gb_get(gb, i) != ' ' && gb_get(gb, i) != '"' && gb_get(gb, i) != '\'' && gb_get(gb, i) != ';' && gb_get(gb, i) != ':')
		{
			if (ci->index == i)
			{
				ci->color = YELLOW_TEXT;
			}
			else
			{
				CI* ci = malloc(sizeof(CI));
				if (ci == NULL)
				{
					log_error("malloc failed in update_color_indices\n");
					return;
				}
				ci->index = i;
				ci->color = YELLOW_TEXT;
				add(line->color_indices, ci, line->color_indices->size);
			}
		}

		if (gb_get(gb, i) == ';' || gb_get(gb, i) == ':')
		{
			if (ci->index == i)
			{
				ci->color = CYAN_TEXT;
			}
			else
			{
				CI* ci = malloc(sizeof(CI));
				if (ci == NULL)
				{
					log_error("malloc failed in update_color_indices\n");
					return;
				}
				ci->index = i;
				ci->color = CYAN_TEXT;
				add(line->color_indices, ci, line->color_indices->size);
			}
		}

		// setting up a string that we can check against different categories of words
		int start_index = ci->index;
		int end_index = i;
		for (; gb_get(gb, start_index) == ' ' || gb_get(gb, start_index) == '\t' || gb_get(gb, start_index) == '*'; start_index++) {}

		for (; gb_get(gb, end_index - 1) == '*'; end_index--) {}

		bool match = false;
		for (int j = 0; j < NUM_DATA_TYPES; j++)
		{
			if (!gb_strcmp(gb, start_index, end_index, data_types[j]))
			{
				match = true;
				break;
			}
		}
		
		// lazy solution for highlighting structs as data types:
		// if it starts with an uppercase but isnt all uppercase, its highlighted like a data type
		if (gb_get(gb, 0) != '\0')
		{
			if (gb_get(gb, 1) != '\0')
			{
				if (gb_get(gb, start_index) >= 'A' && gb_get(gb, start_index) <= 'Z' && gb_get(gb, start_index + 1) >= 'a' && gb_get(gb, start_index + 1) <= 'z')
				{
					match = true;
				}
			}
		}

		if (match)
		{
			ci->color = BLUE_TEXT;
			if (gb_get(gb, i) == '\0')
			{
				break;
			}
			continue;
		}

		for (int j = 0; j < NUM_CONTROL_WORDS; j++)
		{
			if (!gb_strcmp(gb, start_index, end_index, control_words[j]))
			{
				match = true;
				break;
			}
		}

		if (match)
		{
			ci->color = MAGENTA_TEXT;
			if (gb_get(gb, i) == '\0')
			{
				break;
			}
			continue;
		}

		for (int j = 0; j < NUM_LITERALS; j++)
		{
			if (!gb_strcmp(gb, start_index, end_index, literals[j]))
			{
				match = true;
				break;
			}
		}

		if ((gb_get(gb, start_index) >= '0' && gb_get(gb, start_index) <= '9') || gb_get(gb, start_index) == '.')
		{
			match = true;
		}

		if (match)
		{
			ci->color = RED_TEXT;
			if (gb_get(gb, i) == '\0')
			{
				break;
			}
			continue;
		}

		if (ci->color == -1)
		{
			ci->color = CYAN_TEXT;
		}
		if (gb_get(gb, i) == '\0')
		{
			break;
		}
	}
}

void free_line(Line* line)
{
	if (line == NULL)
	{
		return;
	}
	if (line->color_indices != NULL)
	{
		free_list(line->color_indices);
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
