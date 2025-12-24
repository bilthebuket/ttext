#ifndef IO_TOOLS_H
#define IO_TOOLS_H

#include "tab.h"

void print_tab(Tab* t);
void print_line(Tab* t, int line_index);
void print_message(char* str);
void clear_message_line(void);

void move_cursor_to_tab(Tab* t);

void check_left_update(Tab* t);
void check_right_update(Tab* t);
void check_top_update(Tab* t);
void check_bottom_update(Tab* t);

#endif
