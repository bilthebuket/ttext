#ifndef NORMAL_MODE_MOTIONS_H
#define NORMAL_MODE_MOTIONS_H

#include "global.h"

typedef struct Coordinate
{
	int x;
	int y;
} Coordinate;

// returns the same coordinates that active_tab is already at if theres a failure
Coordinate get_target_index(EditorState* es, char motion);
void initialize_normal_mode_motions(void);

#endif
