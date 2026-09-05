#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"
#include "snake.h"
#include "tab.h"
#include "io_tools.h"

static void process_line(PieceIterator* pi, PieceIterator* ci, unsigned char* game, int game_length, int line_size)
{
	srand(time(NULL));
	char c = pt_iterate(pi);
	int color = ci_iterate(ci);
	int index_to_place_chars = 0;
	int room_for_space = game_length - line_size;
	while (c != '\0' && c != '\n')
	{
		int increment = rand() % (room_for_space + 1);
		room_for_space -= increment;
		index_to_place_chars += increment;
		if (index_to_place_chars >= game_length)
		{
			return;
		}
		while (c == ' ')
		{
			c = pt_iterate(pi);
			color = ci_iterate(ci);
		}

		int store_color = color;
		while (color == store_color && c != '\n' && c != '\0')
		{
			game[index_to_place_chars * BYTES_PER_CELL] = c;
			game[index_to_place_chars * BYTES_PER_CELL + 1] = color;
			c = pt_iterate(pi);
			color = ci_iterate(ci);
			index_to_place_chars++;
			if (index_to_place_chars >= game_length)
			{
				return;
			}
		}
	}
}

static void update_index_in_direction(int* index, int direction, int width)
{
	switch (direction)
	{
		case PLAYER_LEFT:
		{
			(*index)--;;
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

// x and y are the coordinates of the top left corner of the tab on the screen
static void print_state(unsigned char* game, int height, int width, int x, int y)
{
	for (int i = 0; i < height; i++)
	{
		for (int j = 0; j < width; j++)
		{
			switch (game[(i * width + j) * BYTES_PER_CELL])
			{
				default:
				{
					attron(COLOR_PAIR(game[(i * width + j) * BYTES_PER_CELL + 1]));
					mvaddch(y + i, x + j, game[(i * width + j) * BYTES_PER_CELL]);
					attroff(COLOR_PAIR(game[(i * width + j) * BYTES_PER_CELL + 1]));
					break;
				}

				case '\0':
				{
					mvaddch(y + i, x + j, ' ');
					break;
				}

				case PLAYER_CHAR:
				{
					attron(A_STANDOUT);
					mvaddch(y + i, x + j, ' ');
					attroff(A_STANDOUT);
					break;
				}
			}
		}
	}

	refresh();
}

void snake_execute(Tab* t)
{
	if (t == NULL)
	{
		return;
	}

	// contains the entire map, each x,y coordinate on the screen maps to a
	// pair of two unsigned chars. if the first char is less than NUM_CHARS, its
	// a char on the screen and the second char will be the color. otherwise that character
	// is a piece of the player's snake, and the second char is the direction its heading in
	// (because we need to keep track of what direction each piece of the player's snake is moving in)
	unsigned char* game = calloc(t->height * t->width * BYTES_PER_CELL, sizeof(char));
	if (game == NULL)
	{
		return;
	}

	int line_index = pt_get_line_index(t->pt, t->top_line_index);
	if (line_index < 0)
	{
		return;
	}

	PieceIterator pi;
	if (!pt_iterator_init(t->pt, &pi, line_index))
	{
		return;
	}

	PieceIterator ci;
	if (!ci_iterator_init(t->pt, &ci, line_index))
	{
		return;
	}

	int total_lines = pt_get_num_lines(t->pt);
	for (int i = t->top_line_index; i < t->top_line_index + t->height && i < total_lines; i++)
	{
		int line_below_index = pt_get_line_index(t->pt, i + 1);
		int line_size;
		if (line_below_index < 0)
		{
			int total_chars = pt_get_size(t->pt);
			if (total_chars < 0)
			{
				break;
			}
			line_size = total_chars - line_index;
		}
		else
		{
			line_size = line_below_index - line_index - 1;
			line_index = line_below_index;
		}
		process_line(&pi, &ci, &game[i * t->width * BYTES_PER_CELL], t->width, line_size);
	}

	int player_index = (t->height / 2) * t->width + (t->width / 2);
	int player_tail_index = player_index;
	game[player_index * BYTES_PER_CELL] = PLAYER_CHAR;
	game[player_index * BYTES_PER_CELL + 1] = PLAYER_UP;

	curs_set(0);
	print_state(game, t->height, t->width, t->xpos, t->ypos);
	print_message("press any key to start, press escape at any time during the game to quit");
	getch();

	nodelay(stdscr, TRUE);

	while (1)
	{
		int ch = getch();

		bool terminate = false;
		switch (ch)
		{
			case 'h':
			{
				game[player_index * BYTES_PER_CELL + 1] = PLAYER_LEFT;
				break;
			}

			case 'j':
			{
				game[player_index * BYTES_PER_CELL + 1] = PLAYER_DOWN;
				break;
			}

			case 'k':
			{
				game[player_index * BYTES_PER_CELL + 1] = PLAYER_UP;
				break;
			}

			case 'l':
			{
				game[player_index * BYTES_PER_CELL + 1] = PLAYER_RIGHT;
				break;
			}

			case ESCAPE_KEYCODE:
			{
				terminate = true;
				break;
			}
		}

		if (terminate)
		{
			break;
		}

		int index_of_facing_char = player_index;
		update_index_in_direction(&index_of_facing_char, game[player_index * BYTES_PER_CELL + 1], t->width);
		if (index_of_facing_char < 0 || index_of_facing_char >= t->height * t->width)
		{
			break;
		}

		if (game[index_of_facing_char * BYTES_PER_CELL] != '\0')
		{
			if (game[index_of_facing_char * BYTES_PER_CELL] == PLAYER_CHAR)
			{
				break;
			}
			else
			{
				game[index_of_facing_char * BYTES_PER_CELL] = PLAYER_CHAR;
				game[index_of_facing_char * BYTES_PER_CELL + 1] = game[player_index * BYTES_PER_CELL + 1];
				player_index = index_of_facing_char;
			}
		}
		else
		{
			game[index_of_facing_char * BYTES_PER_CELL] = PLAYER_CHAR;
			game[index_of_facing_char * BYTES_PER_CELL + 1] = game[player_index * BYTES_PER_CELL + 1];
			player_index = index_of_facing_char;

			int direction = game[player_tail_index * BYTES_PER_CELL + 1];
			game[player_tail_index * BYTES_PER_CELL] = '\0';
			game[player_tail_index * BYTES_PER_CELL + 1] = '\0';
			update_index_in_direction(&player_tail_index, direction, t->width);
		}

		print_state(game, t->height, t->width, t->xpos, t->ypos);
		napms(SNAKE_SLEEP_TIME);
	}

	print_tab(t);
	nodelay(stdscr, FALSE);
	curs_set(1);
	refresh();
	clear_message_line();
}
