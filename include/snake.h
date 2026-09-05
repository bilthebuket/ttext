#ifndef SNAKE_H
#define SNAKE_H

#define PLAYER_CHAR 128
#define PLAYER_UP 0
#define PLAYER_LEFT 1
#define PLAYER_RIGHT 2
#define PLAYER_DOWN 3

#define BYTES_PER_CELL 2

#define SNAKE_SLEEP_TIME 250

#include "tab.h"

void snake_execute(Tab* t);

#endif
