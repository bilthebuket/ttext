#ifndef NORMAL_MODE_NORMAL_MODE_H
#define NORMAL_MODE_NORMAL_MODE_H

#include "global.h"

void normal_mode(EditorState* es, int ch);
void normal_mode_create(void);

void handle_rm_on_boundary(EditorState* es, int start_index, int end_index);
bool is_motion(char c);

#endif
