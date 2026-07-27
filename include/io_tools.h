#ifndef IO_TOOLS_H
#define IO_TOOLS_H

#include <stdbool.h>
#include "tab.h"
#include "line.h"
#include "global.h"
#include "piece_table/piece_table.h"

void print_tab(Tab* t);
void print_line(Tab* t, int line_index);
void print_message(const char* const str);
void clear_message_line(void);
void print_screen(EditorState* es);
void set_tab_to_fill_screen(Tab* t);

void screen_create(void);
void screen_free(void);

bool is_tab_on_screen(Tab* t);

void move_cursor_to_tab(Tab* t);

void check_left_update(Tab* t);
void check_right_update(Tab* t);
void check_top_update(Tab* t);
void check_bottom_update(Tab* t);

void convert_tabs_to_spaces(GapBuffer* gb);
int indent_line(EditorState* es, Tab* t, int index);

void move_cursor_to_valid_coordinates(Tab* t);

void log_error(const char* str);
void print_pt_to_message_bar(PieceTable* pt);

// checks for values like "w", "w2", "w3", "h", "h4" and replaces them with the actual constants
// used for screen movement and resizing in terminal mode
char* parse_screen_values(char* str);

#endif
