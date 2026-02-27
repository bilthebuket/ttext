#ifndef TREE_H
#define TREE_H

#include <stdbool.h>

typedef struct Tree
{
	void* elt;
	struct Tree* left;
	struct Tree* right;
	struct Tree* prev;
	int height;
} Tree;

Tree* tree_helper(Tree* t, void* elt, int (*cmp)(Tree*, void*));
Tree* tree_create(void* elt);
Tree* tree_add_elt(Tree* t, void* elt, int (*cmp)(Tree*, void*), void (*update_relative_info)(Tree*));
Tree* tree_add_tree(Tree* t, Tree* to_add, int (*cmp)(Tree*, void*), void (*update_relative_info)(Tree*), bool balance);
Tree* tree_rm(Tree* t, void* elt, int (*cmp)(Tree*, void*), void (*free_node)(void*), void (*update_relative_info)(Tree*));
void* tree_get(Tree* t, void* elt, int (*cmp)(Tree*, void*));
void tree_free(Tree* t, void (*free_node)(void*));
Tree* tree_balance(Tree* t, int (*cmp)(Tree*, void*), void (*update_relative_info)(Tree*));

// only updates the height of t, not any of the nodes connected to it
void update_height(Tree* t, void (*update_relative_info)(Tree*));

// calls update_relative_info on t and calls recursively on t->prev
void recursive_update_to_root(Tree* t, void (*update_relative_info)(Tree*));

// returns pointer to tree that replaced the tree that got rotated
Tree* tree_rotate(Tree* t, void (*update_relative_info)(Tree*));

void print_tree(Tree* t, bool reset, int row, int col);
void print_info(Tree* t, void (*print_elt)(void*));
bool tree_find(Tree* t, Tree* to_find);

#endif
