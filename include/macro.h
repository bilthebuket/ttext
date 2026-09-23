#ifndef MACRO_H
#define MACRO_H

#include <stdbool.h>
#include "dynamic_array.h"

#define NUM_MACROS 10

typedef struct MacroBundle
{
	char* macros[NUM_MACROS];
	DynamicArray* macro_recording;
	char target;
} MacroBundle;

#include "global.h"

void macro_mode(EditorState* es, int ch);
void execute_macro(EditorState* es);
void mb_init(MacroBundle* mb);
void mb_uninit(MacroBundle* mb);
bool valid_macro_assignment(EditorState* es);

#endif
