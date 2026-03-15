#ifndef TERMINAL_MODE_H
#define TERMINAL_MODE_H

#include "global.h"
#include <stdbool.h>

void terminal_mode(EditorState* es, int ch);
void* listener_func(void* v);
bool terminal_create(EditorState* es);
void terminal_free(EditorState* es);

void print_terminal(void);
void move_cursor_to_terminal(void);

#endif
