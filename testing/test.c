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
		//t = tree_add_elt(t, p, &compare, NULL);
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
		printf("./test <fname>\n");
		return -1;
	}
	int len = 0;
	for (; argv[1][len] != '\0'; len++) {}
	char* buf = malloc(sizeof(char) * len);
	for (int i = 0; i < len; i++)
	{
		buf[i] = argv[1][i];
	}
	Tab* t = make_tab(buf);
	int index = pt_get_line_index(t->pt, 1);
	pt_rm(t->pt, index + 2);
	index = pt_get_line_index(t->pt, 0);
	index = pt_get_line_index(t->pt, 1);
	index = pt_get_line_index(t->pt, 2);
	print_info(t->pt->pieces, &print_piece);
	free_tab(t);
}
