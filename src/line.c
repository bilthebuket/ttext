#include <stdlib.h>
#include <string.h>
#include "line.h"
#include "global.h"

void update_color_indices(Line* line)
{
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
		add(line->color_indices, ci, line->color_indices->size);
		ci->index = i;
		for (; line->text[i] != '\0' && line->text[i] != ' ' && line->text[i] != '('; i++)
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
			continue;
		}
		if (line->text[i] == '(')
		{
			ci->color = YELLOW_TEXT;
			continue;
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

		ci->color = CYAN_TEXT;
		if (store == '\0')
		{
			break;
		}
		line->text[i] = store;
	}
}

void free_line(Line* line)
{
	free_list(line->color_indices);
	free(line->text);
	free(line);
}
