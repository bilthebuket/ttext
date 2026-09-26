#include <ncurses.h>
#include "game.h"

void print_char(unsigned char* game, int row, int col, int x, int y, int width)
{
	switch (game[(row * width + col) * BYTES_PER_CELL])
	{
		default:
		{
			attron(COLOR_PAIR(game[(row * width + col) * BYTES_PER_CELL + 1]));
			mvaddch(y + row, x + col, game[(row * width + col) * BYTES_PER_CELL]);
			attroff(COLOR_PAIR(game[(row * width + col) * BYTES_PER_CELL + 1]));
			break;
		}

		case '\0':
		{
			mvaddch(y + row, x + col, ' ');
			break;
		}

		case PLAYER_CHAR:
		{
			attron(A_STANDOUT);
			mvaddch(y + row, x + col, ' ');
			attroff(A_STANDOUT);
			break;
		}
	}
}

// x and y are the coordinates of the top left corner of the tab on the screen
void print_state(unsigned char* game, int height, int width, int x, int y)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			print_char(game, i, j, x, y, width);
		}
	}

	refresh();
}

void update_index_in_direction(int* index, int direction, int width)
{
	switch (direction)
	{
		case PLAYER_LEFT:
		{
			(*index)--;
			break;
		}

		case PLAYER_RIGHT:
		{
			(*index)++;
			break;
		}

		case PLAYER_UP:
		{
			*index -= width;
			break;
		}

		case PLAYER_DOWN:
		{
			*index += width;
			break;
		}
	}
}
