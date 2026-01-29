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
	if (line->text == NULL)
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

	// after every iteration we need to increment i
	// however, there are a lot of continue statements, so instead of putting i++ before every continue,
	// i have it at the top. this means for the first iteration we need to start at i = -1 so it gets incremented to 0
	int i = -1;
	while (1)
	{
		i++;
		ColorIndex* ci = malloc(sizeof(ColorIndex));
		if (ci == NULL)
		{
			log_error("malloc failed in update_color_indices\n");
			return;
		}
		add(line->color_indices, ci, line->color_indices->size);
		ci->index = i;
		ci->color = -1;
		for (; line->text[i] != '\0' && line->text[i] != ' ' && line->text[i] != '(' && line->text[i] != ')' && line->text[i] != '[' && line->text[i] != ']' && 
		line->text[i] != '{' && line->text[i] != '}' && line->text[i] != '\'' && line->text[i] != '"' && line->text[i] != ';' && line->text[i] != ':'; i++)
		{
			if (line->text[i] == '/')
			{
				if (line->text[i + 1] == '/')
				{
					break;
				}
			}
		}

		if (line->text[i] == '/')
		{
			ci->color = GREEN_TEXT;
			break;
		}
		if (line->text[i] == '(')
		{
			ci->color = YELLOW_TEXT;
			continue;
		}

		if (line->text[i] == '\'' || line->text[i] == '"')
		{
			// even if theres spaces inside the quotes we want everything highlighted red, so this skips i to outside the quotes

			if (i == ci->index)
			{
				ci->color = RED_TEXT;
			}
			else
			{
				ColorIndex* ci = malloc(sizeof(ColorIndex));
				if (ci == NULL)
				{
					log_error("malloc failed in update_color_indices\n");
					return;
				}
				ci->index = i;
				ci->color = RED_TEXT;
				add(line->color_indices, ci, line->color_indices->size);
			}
			char c = line->text[i];
			i++;
			for (; line->text[i] != c && line->text[i] != '\0'; i++) {}
			if (line->text[i] == '\0')
			{
				break;
			}
		}

		// this if statement makes it so all grouping symbols except quotes are yellow, but then we still need to find out what color to highlight
		// the text that precedes it
		if (line->text[i] != '\0' && line->text[i] != ' ' && line->text[i] != '"' && line->text[i] != '\'' && line->text[i] != ';' && line->text[i] != ':')
		{
			if (ci->index == i)
			{
				ci->color = YELLOW_TEXT;
			}
			else
			{
				ColorIndex* ci = malloc(sizeof(ColorIndex));
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

		if (line->text[i] == ';' || line->text[i] == ':')
		{
			if (ci->index == i)
			{
				ci->color = CYAN_TEXT;
			}
			else
			{
				ColorIndex* ci = malloc(sizeof(ColorIndex));
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
		char* ptr = &line->text[ci->index];
		char store = line->text[i];
		line->text[i] = '\0';
		for (; ptr[0] == ' ' || ptr[0] == '\t' || ptr[0] == '*'; ptr++) {}
		int k;
		for (k = 0; ptr[k] != '\0'; k++) {}
		k--;
		if (k >= 0)
		{
			for (; ptr[k] == '*'; k--)
			{
				ptr[k] = '\0';
			}
		}

		bool match = false;
		for (int j = 0; j < NUM_DATA_TYPES; j++)
		{
			if (!strcmp(ptr, data_types[j]))
			{
				match = true;
				break;
			}
		}
		
		// lazy solution for highlighting structs as data types:
		// if it starts with an uppercase but isnt all uppercase, its highlighted like a data type
		if (ptr[0] != '\0')
		{
			if (ptr[1] != '\0')
			{
				if (ptr[0] >= 'A' && ptr[0] <= 'Z' && ptr[1] >= 'a' && ptr[1] <= 'z')
				{
					match = true;
				}
			}
		}

		k++;
		if (k >= 0)
		{
			for (; ptr[k] == '\0' && &ptr[k] != &line->text[i]; k++)
			{
				ptr[k] = '*';
			}
		}

		if (match)
		{
			ci->color = BLUE_TEXT;
			if (store == '\0')
			{
				break;
			}
			line->text[i] = store;
			continue;
		}

		for (int j = 0; j < NUM_CONTROL_WORDS; j++)
		{
			if (!strcmp(ptr, control_words[j]))
			{
				match = true;
				break;
			}
		}

		if (match)
		{
			ci->color = MAGENTA_TEXT;
			if (store == '\0')
			{
				break;
			}
			line->text[i] = store;
			continue;
		}

		for (int j = 0; j < NUM_LITERALS; j++)
		{
			if (!strcmp(ptr, literals[j]))
			{
				match = true;
				break;
			}
		}

		if ((ptr[0] >= '0' && ptr[0] <= '9') || ptr[0] == '.')
		{
			match = true;
		}

		if (match)
		{
			ci->color = RED_TEXT;
			if (store == '\0')
			{
				break;
			}
			line->text[i] = store;
			continue;
		}

		if (ci->color == -1)
		{
			ci->color = CYAN_TEXT;
		}
		if (store == '\0')
		{
			break;
		}
		line->text[i] = store;
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
	if (line->text != NULL)
	{
		free(line->text);
	}
	free(line);
}
