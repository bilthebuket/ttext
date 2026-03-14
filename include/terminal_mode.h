#ifndef TERMINAL_MODE_H
#define TERMINAL_MODE_H

#include "tab.h"
#include <stdbool.h>

Tab* terminal_mode(Tab* t, int ch);
void* listener_func(void*);
bool init_terminal(void);
void free_terminal(void);

void print_terminal(void);
void move_cursor_to_terminal(void);

#endif
