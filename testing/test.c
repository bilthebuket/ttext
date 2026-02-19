#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "line.h"
#include "global.h"
#include "tree.h"

#define SIZE 200

int power(int x, int y)
{
	int r = x;
	for (int i = 0; i < y; i++)
	{
		r *= r;
	}
	return r;
}

int compare(void* one, void* two)
{
	int* x = (int*) one;
	int* y = (int*) two;
	if (x == NULL || y == NULL)
	{
		printf("NULL pointer in compare function\n");
		exit(1);
	}
	if (*y > *x)
	{
		return 1;
	}
	else if (*y < *x)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void free_thing(void* v)
{
	free(v);
}

void print_tree(Tree* t, char lines[][SIZE], int row, int col)
{
	if (t == NULL)
	{
		return;
	}
	lines[row][col + (SIZE / 2)] = (*(int*) t->elt) + 'a';
	print_tree(t->left, lines, row + 1, col - (SIZE / (3 * (row + 1))));
	print_tree(t->right, lines, row + 1, col + (SIZE / (3 * (row + 1))));
	if (row == 0)
	{
		for (int i = 0; i < SIZE; i++)
		{
			lines[i][SIZE - 1] = '\0';
			printf("%s\n", lines[i]);
		}
	}
}

int main(int argc, char* argv[])
{
	int* x = malloc(sizeof(int));
	*x = 20;
	Tree* t = tree_create(x);
	for (int i = 0; i < 20; i++)
	{
		int* p = malloc(sizeof(int));
		*p = i;
		t = tree_add_elt(t, p, &compare);
	}
	int y = 0;
	t = tree_rm(t, &y, &compare, &free_thing);
	y = 12;
	t = tree_rm(t, &y, &compare, &free_thing);
	char lines[SIZE][SIZE];
	for (int i = 0; i < SIZE; i++)
	{
		for (int j = 0; j < SIZE; j++)
		{
			lines[i][j] = ' ';
		}
	}
	print_tree(t, lines, 0, 0);
}
