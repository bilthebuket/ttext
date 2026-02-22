#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include "line.h"
#include "global.h"
#include "tree.h"
#include "piece_table.h"

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

void test_tree(void)
{
	bool arr[21];
	for (int i = 0; i < 20; i++)
	{
		arr[i] = true;
	}
	srand(time(NULL));
	arr[20] = false;

	int* x = malloc(sizeof(int));
	*x = 20;
	Tree* t = tree_create(x);
	for (int i = 0; i < 20; i++)
	{
		int* p = malloc(sizeof(int));
		int y = rand() % 21;
		while (!arr[y])
		{
			y = rand() % 21;
		}
		arr[y] = false;
		printf("adding next: %c\n", 'a' + y);
		*p = y;
		t = tree_add_elt(t, p, &compare, NULL);
		print_tree(t, true, 0, 0);
		fflush(stdout);
	}

	/*
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
	*/
	tree_free(t, free_thing);
}

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		return -1;
	}
	int len = 0;
	for (; argv[1][len] != '\0'; len++) {}
	char* buf = malloc(sizeof(char) * len);
	for (int i = 0; i < len; i++)
	{
		buf[i] = argv[1][i];
	}
	PieceTable* pt = pt_create(buf, len);
	pt_insert(pt, 'z', len);
	pt_insert(pt, 'a', len + 1);
	pt_insert(pt, 'z', len + 2);
	pt_rm(pt, len + 2);
	for (int i = 0; pt_get(pt, i) != '\0'; i++)
	{
		printf("%c", pt_get(pt, i));
	}
	printf("\n");
	pt_free(pt);
}
