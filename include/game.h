#ifndef GAME_H
#define GAME_H

#define PLAYER_CHAR 128
#define PLAYER_UP 0
#define PLAYER_LEFT 1
#define PLAYER_RIGHT 2
#define PLAYER_DOWN 3

#define BYTES_PER_CELL 2

#define SNAKE_SLEEP_TIME 250

void print_state(unsigned char* game, int height, int width, int x, int y);
void print_char(unsigned char* game, int row, int col, int x, int y, int width);
void update_index_in_direction(int* index, int direction, int width)

#endif
