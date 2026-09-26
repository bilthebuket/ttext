#include <stdlib.h>
#include <ncurses.h>
#include "brickbreaker.h"
#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"
#include "global.h"
#include "io_tools.h"

static void process_line(PieceIterator* pi, PieceIterator* ci, unsigned char* game, int game_length, int line_size)
{
	char c = pt_iterate(pi);
	int color = ci_iterate(ci);
	int offset = (game_length - line_size) / 2;
	while (1)
	{
		int store_color = color;
		for (; color == store_color && c != ' ' && c != '\n' && c != '\0'; c = pt_iterate(pi), color = ci_iterate(ci))
		{
			game[BYTES_PER_CELL * offset] = c;
			game[BYTES_PER_CELL * offset + 1] = color;
		}
		for (; c == ' ' || c == '\n'; c = pt_iterate(pi), color = ci_iterate(ci)) {}

		if (offset >= line_size || offset >= game_length)
		{
			break;
		}
	}
}

// x and y are the coordinates of the top left corner of the tab on the screen
static void print_char(unsigned char* game, int row, int col, int x, int y, int width)
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

		case BALL_CHAR:
		{
			attron(A_STANDOUT);
			mvaddch(y + row, x + col, ' ');
			attroff(A_STANDOUT);
			break;
		}
	}
}

static void print_state(unsigned char* game, int height, int width, int x, int y)
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

void brickbreaker_execute(Tab* t)
{
	if (t == NULL)
	{
		return;
	}

	unsigned char* game = calloc(t->width * t->height * BYTES_PER_CELL, sizeof(char));
	if (game == NULL)
	{
		return;
	}

	for (int i = 0; i < t->height / 2; i++)
	{
		int index_we_are_on = pt_get_line_index(t->pt, t->top_line_index + i);
		PieceIterator pi;
		PieceIterator ci;
		if (!pt_iterator_init(t->pt, &pi, index_we_are_on) || !ci_iterator_init(t->pt, &ci, index_we_are_on))
		{
			break;
		}

		int line_size = 0;
		char c = pt_iterate(&pi);
		for (int j = 0; j < t->width && c != '\n' && c != '\0'; j++, c = pt_iterate(&pi))
		{
			if (c != ' ')
			{
				line_size++;
			}
		}

		if (!pt_iterator_init(t->pt, &pi, index_we_are_on))
		{
			break;
		}
		process_line(&pi, &ci, game, t->width, line_size);
	}

	int player_index = t->width * (t->height - 1) + t->width / 2;
	int ball_index = t->width * (t->height - 2) + t->width / 2;
	int ball_y_vel = -1;
	int ball_x_vel = 1;
	bool terminate = false;

	for (int i = PLAYER_WIDTH / -2; i <= PLAYER_WIDTH / 2; i++)
	{
		game[(player_index + i) * BYTES_PER_CELL] = PLAYER_CHAR;
	}
	game[ball_index * BYTES_PER_CELL] = BALL_CHAR;

	curs_set(0);
	print_state(game, t->height, t->width, t->xpos, t->ypos);
	print_message("Press any key to start, press escape at any time quit");
	getch();

	nodelay(stdscr, TRUE);

	while (!terminate)
	{
		int ch = getch();
		switch (ch)
		{
			case 'h':
			{
				if (player_index - PLAYER_WIDTH / 2 > t->width * (t->height - 1))
				{
					game[(player_index + PLAYER_WIDTH / 2) * BYTES_PER_CELL] = '\0';
					game[(player_index - PLAYER_WIDTH / 2 - 1) * BYTES_PER_CELL] = PLAYER_CHAR;
					print_char(game, (player_index + PLAYER_WIDTH / 2) / t->width, (player_index + PLAYER_WIDTH / 2) % t->width, t->xpos, t->ypos, t->width);
					print_char(game, (player_index - PLAYER_WIDTH / 2 - 1) / t->width, (player_index - PLAYER_WIDTH / 2 - 1) % t->width, t->xpos, t->ypos, t->width);
					player_index--;
				}
				break;
			}
			case 'l':
			{
				if (player_index + PLAYER_WIDTH / 2 + 1 < t->width * t->height)
				{
					game[(player_index - PLAYER_WIDTH / 2) * BYTES_PER_CELL] = '\0';
					game[(player_index + PLAYER_WIDTH / 2 + 1) * BYTES_PER_CELL] = PLAYER_CHAR;
					print_char(game, (player_index - PLAYER_WIDTH / 2) / t->width, (player_index - PLAYER_WIDTH / 2) % t->width, t->xpos, t->ypos, t->width);
					print_char(game, (player_index + PLAYER_WIDTH / 2 + 1) / t->width, (player_index + PLAYER_WIDTH / 2 + 1) % t->width, t->xpos, t->ypos, t->width);
					player_index++;
				}
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

		bool collision = false;
		if (abs((ball_index + ball_x_vel) % t->width - ball_index % t->width) > 1)
		{
			ball_x_vel *= -1;
			collision = true;
		}
		if (ball_index + ball_y_vel * t->width < 0 || ball_index + ball_y_vel * t->width >= t->height * t->width)
		{
			ball_y_vel *= -1;
			collision = true;
		}

		if (!collision)
		{
			if (game[(ball_index + ball_y_vel * t->width + ball_x_vel) * BYTES_PER_CELL] == PLAYER_CHAR)
			{
				ball_x_vel *= -1;
				ball_y_vel *= -1;
			}
			else if (game[(ball_index + ball_y_vel * t->width + ball_x_vel) * BYTES_PER_CELL] != '\0')
			{
				game[(ball_index + ball_y_vel * t->width + ball_x_vel) * BYTES_PER_CELL] = '\0';
				print_char(game, ball_index / t->width + ball_y_vel, ball_index % t->width + ball_x_vel, t->xpos, t->ypos, t->width);
				ball_x_vel *= -1;
				ball_y_vel *= -1;
			}
		}

		game[ball_index * BYTES_PER_CELL] = '\0';
		print_char(game, ball_index / t->width, ball_index % t->width, t->xpos, t->ypos, t->width);

		ball_index = ball_index + ball_y_vel * t->width + ball_x_vel;
		game[ball_index * BYTES_PER_CELL] = BALL_CHAR;
		print_char(game, ball_index / t->width, ball_index % t->width, t->xpos, t->ypos, t->width);

		refresh();
		napms(SLEEP_TIME);
	}

	free(game);
	print_tab(t);
	nodelay(stdscr, FALSE);
	curs_set(1);
	refresh();
	clear_message_line();
}
