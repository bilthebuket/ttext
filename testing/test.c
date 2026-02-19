#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "line.h"
#include "global.h"
#include "tree.h"

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

int main(int argc, char* argv[])
{
	int* x = malloc(sizeof(int));
	*x = 20;
	Tree* t = tree_create(x);
	for (int i = 0; i < 20; i++)
	{
		int* p = malloc(sizeof(int));
		*p = i;
		t = tree_add_elt(t, p, &compare, NULL);
	}
	bool arr[21];
	for (int i = 0; i < 21; i++)
	{
		arr[i] = false;
	}

	srand(time(NULL));
	for (int i = 0; i <= 20; i++)
	{
		int y = rand() % 21;
		while (arr[y])
		{
			y = rand() % 21;
		}
		arr[y] = true;
		printf("removing next: %c\n", 'a' + y);
		t = tree_rm(t, &y, &compare, &free_thing, NULL);

		print_tree(t, true, 0, 0);
		fflush(stdout);
	}
	tree_free(t, free_thing);
}
