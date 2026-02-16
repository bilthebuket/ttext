#include <stdio.h>
#include <stdlib.h>
#include "line.h"
#include "global.h"
#include "tree.h"

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

void free(void* v)
{
	return;
}

int main(int argc, char* argv[])
{
	int x = 3;
	int y = 2;
	int z = 1;
	Tree* t = tree_create(&x);
	tree_add_elt(t, &y, &compare);
	tree_add_elt(t, &z, &compare);

	printf("  %d\n", *((int*) t->elt));
	printf(" %d %d\n", *((int*) t->left->elt), *((int*) t->right->elt));
}
