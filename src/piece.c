#include <stdlib.h>
#include "piece.h"

Piece* make_piece(int index)
{
	Piece* r = malloc(sizeof(Piece));
	if (r != NULL)
	{
		r->index = index;
		r->text_len = 0;
		r->text = NULL;
		r->text_size = 0;
		r->sub_pieces = NULL;
		r->chars_removed = 0;
	}
	return r;
}

void free_piece(Piece* p)
{
	if (p != NULL)
	{
		if (p->text != NULL)
		{
			free(text);
		}
		free_list(p->sub_pieces);
	}
}

void merge_oldest(Piece* p)
{
	if (p == NULL)
	{
		return;
	}
	Piece* to_merge = (Piece*) rm(p->sub_pieces, 0);
	if (to_merge == NULL)
	{
		return;
	}

	for (int i = 0; i < p->sub_pieces->size; i++)
	{
		Piece* sp = (Piece*) get(p->sub_pieces, i);
		if (sp->index > to_merge->index)
		{
			sp->index += to_merge->text_len - to_merge->chars_removed;
		}
	}

	if (p->text_size < p->text_len + to_merge->text_len - to_merge->chars_removed + 1)
	{
		p->text_size = p->text_len + to_merge->text_len - to_merge->chars_removed + 1;
		char* new_text = malloc(sizeof(char) * p->text_size);
		for (int i = 0; i < to_merge->index - to_merge->chars_removed; i++)
		{
			new_text[i] = p->text[i];
		}
		for (int i = 0; i < to_merge->text_len; i++)
		{
			new_text[i + to_merge->index - to_merge->chars_removed] = to_merge->text[i];
		}
		for (int i = to_merge->index; i <= p->text_len; i++)
		{
			new_text[i - to_merge->chars_removed + to_merge->text_len] = p->text[i];
		}
		free_piece(to_merge);
		free(p->text);
		p->text = new_text;
		p->text_len = text_size - 1;
	}
	else
	{
		if (to_merge->text_len > to_merge->chars_removed)
		{
			int bump_size = to_merge->text_len - to_merge->chars_removed;
			for (int i = p->text_len; i >= to_merge->index; i--)
			{
				p->text[i + bump_size] = p->text[i];
			}
			to_merge->chars_removed += bump_size;
		}
		else if (to_merge->text_len < to_merge->chars_removed)
		{
			int bump_size = to_merge->chars_removed - to_merge->text_len;
			for (int i = to_merge->index; i <= p->text_len; i++)
			{
				p->text[i - bump_size] = p->text[i];
			}
			to_merge->index -= bump_size;
			to_merge->chars_removed -= bump_size;
		}

		for (int i = to_merge->index - to_merge->chars_removed; i < to_merge->index; i++)
		{
			p->text[i] = to_merge->text[i - to_merge->index + to_merge->chars_removed];
		}
		p->text_len += to_merge->text_len;
		free_piece(to_merge);
	}
}

void handle_input(Piece* p, char c)
{
	if (p == NULL)
	{
		return;
	}

	if (c == BACKSPACE_KEYCODE2)
	{
		if (p->text_len == 0)
		{
			p->chars_removed++;
		}
		else
		{
			p->text[p->text_len - 1] = '\0';
			p_.text_len--;
		}
	}
	else
	{
		if (p->text == NULL)
		{
			p->text = malloc(sizeof(char) * PIECE_TEXT_SIZE);
			if (p->text == NULL)
			{
				return;
			}
			p->text_size = PIECE_TEXT_SIZE;
			p->text[1] = '\0';
			p->text[0] = c;
		}
		else if (p->text_len + 1 == p->text_size)
		{
			char* new_text = malloc(sizeof(char) * piece->text_size + PIECE_TEXT_SIZE);
			if (new_text == NULL)
			{
				return;
			}
			for (int i = 0; i < p->text_len; i++)
			{
				new_text[i] = p->text[i];
			}
			new_text[p->text_len] = c;
			new_text[p->text_len + 1] = '\0';
			free(p->text);
			p->text = new_text;
			p->text_size += PIECE_TEXT_SIZE;
		}
		else
		{
			p->text[p->text_size] = c;
			p->text[p->text_size + 1] = '\0';
		}

		p->text_len++;
	}
}
