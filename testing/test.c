#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "line.h"
#include "global.h"
#include "tree.h"
#include "piece_table.h"
#include <criterion/criterion.h>

Tree* t = NULL;
int arr[] = {1, 20, 9, 5, 12, 25, 67, 13, 10, 32};
#define ARR_LENGTH 10

int cmp(Tree*, void*);

int cmp(Tree* t, void* v)
{
	int node = *((int*) t->elt);
	int to_add = *((int*) v);

	if (to_add > node)
	{
		return 1;
	}
	else if (to_add < node)
	{
		return -1;
	}
	else
	{
		return 0;
	}
}

void setup_tree(void)
{
	for (int i = 0; i < ARR_LENGTH; i++)
	{
		int* ptr = malloc(sizeof(int));
		*ptr = arr[i];
		t = tree_add_elt(t, ptr, &cmp, NULL);
	}
}

void teardown_tree(void)
{
	tree_free(t, &free);
}

Test(tree, test_get, .init = setup_tree, .fini = teardown_tree)
{
	for (int i = 0; i < ARR_LENGTH; i++)
	{
		int* ptr = (int*) tree_get(t, &arr[i], &cmp);
		cr_expect_eq(*ptr, arr[i]);
	}
}

Test(tree, test_balancing, .init = setup_tree, .fini = teardown_tree)
{
	for (int i = 0; i < ARR_LENGTH; i++)
	{
		Tree* n = tree_helper(t, &arr[i], &cmp);
		int r_height = 0;
		int l_height = 0;
		if (n->left != NULL)
		{
			l_height = n->left->height + 1;
		}
		if (n->right != NULL)
		{
			r_height = n->right->height + 1;
		}
		cr_expect_leq(abs(l_height - r_height), 1);
		if (l_height == 0 && r_height == 0)
		{
			cr_expect_eq(n->height, 0);
		}
		else if (l_height >= r_height)
		{
			cr_expect_eq(n->height, l_height);
		}
		else
		{
			cr_expect_eq(n->height, r_height);
		}
	}
}

Test(tree, test_rm, .init = setup_tree)
{
	for (int i = 0; i < ARR_LENGTH; i++)
	{
		t = tree_rm(t, &arr[i], &cmp, &free, NULL);
		void* elt = tree_get(t, &arr[i], &cmp);
		cr_expect_eq(elt, NULL);
	}
	cr_expect_eq(t, NULL);
}
