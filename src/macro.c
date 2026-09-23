#include <stdlib.h>
#include "macro.h"
#include "normal_mode/normal_mode.h"
#include "global.h"
#include "io_tools.h"

bool valid_macro_assignment(EditorState* es)
{
	es->mb.target = es->target;
	return es->target >= '0' && es->target <= '9';
}

static int convert_char_to_index(char c)
{
	if (c >= '0' && c <= '9')
	{
		if (c == '0')
		{
			return NUM_MACROS - 1;
		}
		else
		{
			return c - '0' - 1;
		}
	}
	return -1;
}

void macro_mode(EditorState* es, int ch)
{
	switch (ch)
	{
		default:
		{
			da_insert(es->mb.macro_recording, (char) ch, es->mb.macro_recording->len - 1);
			print_message(es->mb.macro_recording->arr);
			break;
		}

		case CTRL_G:
		{
			char* macro = malloc(sizeof(char) * es->mb.macro_recording->len);
			if (macro != NULL)
			{
				for (int i = 0; i < es->mb.macro_recording->len; i++)
				{
					macro[i] = da_get(es->mb.macro_recording, i);
				}
				es->mb.macro_recording->len = 0;
				da_insert(es->mb.macro_recording, '\0', 0);
				int index = convert_char_to_index(es->mb.target);
				if (index >= 0)
				{
					if (es->mb.macros[index] != NULL)
					{
						free(es->mb.macros[index]);
					}
					es->mb.macros[index] = macro;
				}
			}

			es->mode = &normal_mode;
			print_message("Normal Mode");
			break;
		}
	}
}

void execute_macro(EditorState* es)
{
	int index = convert_char_to_index(es->target);
	if (index >= 0)
	{
		es->macro = es->mb.macros[index];
	}
}

void mb_init(MacroBundle* mb)
{
	for (int i = 0; i < NUM_MACROS; i++)
	{
		mb->macros[i] = NULL;
	}
	mb->macro_recording = da_create(-1);
	da_insert(mb->macro_recording, '\0', 0);
	mb->target = '\0';
}

void mb_uninit(MacroBundle* mb)
{
	for (int i = 0; i < NUM_MACROS; i++)
	{
		if (mb->macros[i] != NULL)
		{
			free(mb->macros[i]);
		}
	}
	da_free(mb->macro_recording);
}
