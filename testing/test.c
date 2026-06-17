#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "line.h"
#include "global.h"
#include "tree.h"
#include "piece_table/piece_table.h"
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

PieceTable* pt;
char* text;
int size;

void setup_pt(void)
{
	FILE* f = fopen("testing/test2.txt", "r");
	cr_assert_not_null(f);
	fseek(f, 0, SEEK_END);
	size = ftell(f);
	rewind(f);
	text = malloc(sizeof(char) * size);
	char* text2 = malloc(sizeof(char) * size);
	fread(text, sizeof(char), size, f);
	rewind(f);
	fread(text2, sizeof(char), size, f);
	pt = pt_create(text2, size);
	fclose(f);
}

void teardown_pt(void)
{
	pt_free(pt);
	free(text);
}

Test(piece_table, test_get, .init = setup_pt, .fini = teardown_pt)
{
	for (int i = 0; i < size; i++)
	{
		cr_expect_eq(text[i], pt_get(pt, i));
	}
}

Test(piece_table, test_rm, .init = setup_pt, .fini = teardown_pt)
{
	for (int i = 0; i < size - 1; i++)
	{
		text[i] = text[i + 1];
	}
	pt_rm(pt, 0);

	for (int i = 4; i < size - 2; i++)
	{
		text[i] = text[i + 1];
	}
	pt_rm(pt, 4);

	for (int i = 9; i < size - 3; i++)
	{
		text[i] = text[i + 1];
	}
	pt_rm(pt, 9);

	for (int i = 2; i < size - 4; i++)
	{
		text[i] = text[i + 1];
	}
	pt_rm(pt, 2);

	pt_rm(pt, size - 5);

	for (int i = 0; i < size - 5; i++)
	{
		cr_expect_eq(text[i], pt_get(pt, i));
	}
}

Test(piece_table, test_insert, .init = setup_pt, .fini = teardown_pt)
{
	char* new_text = malloc(sizeof(char) * (size + 4));
	for (int i = 0; i < size; i++)
	{
		new_text[i] = text[i];
	}

	for (int i = size; i > 0; i--)
	{
		new_text[i] = new_text[i - 1];
	}
	new_text[0] = 'c';
	pt_insert(pt, 'c', 0);

	for (int i = size + 1; i > 4; i--)
	{
		new_text[i] = new_text[i - 1];
	}
	new_text[4] = 'a';
	pt_insert(pt, 'a', 4);

	for (int i = size + 2; i > 6; i--)
	{
		new_text[i] = new_text[i - 1];
	}
	new_text[6] = 'm';
	pt_insert(pt, 'm', 6);

	new_text[size + 3] = '2';
	pt_insert(pt, '2', size + 3);

	for (int i = 0; i < size + 4; i++)
	{
		cr_expect_eq(new_text[i], pt_get(pt, i));
	}
	free(new_text);
}
