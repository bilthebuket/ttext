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
Tree* tree_insert(Tree* t, void* elt, int (*cmp)(Tree*, void*), void (*update_relative_info)(Tree*));
Tree* tree_insert_tree(Tree* t, Tree* to_add, int (*cmp)(Tree*, void*), void (*update_relative_info)(Tree*), bool balance);
Tree* tree_rm(Tree* t, void* elt, int (*cmp)(Tree*, void*), void (*free_node)(void*), void (*update_relative_info)(Tree*));
void* tree_get(Tree* t, void* elt, int (*cmp)(Tree*, void*));
void tree_free(Tree* t, void (*free_node)(void*));
Tree* tree_balance(Tree* t, int (*cmp)(Tree*, void*), void (*update_relative_info)(Tree*));

// i wrote this so could i traverse the piece table tree to find out how many characters are in the entire tree.
// then after i wrote the implementation, using a while loop instead of recursion to save memory, i realised that i am stupid,
// and ((Piece*) pt->pieces->elt)->chars_contained already has what i need
void traverse_all(Tree* t, void* place_to_store, void (*thing_to_do)(Tree*, void*));

// only updates the height of t, not any of the nodes connected to it
void tree_update_height(Tree* t, void (*update_relative_info)(Tree*));

// calls update_relative_info on t and calls recursively on t->prev
void tree_recursive_update_to_root(Tree* t, void (*update_relative_info)(Tree*));

// returns pointer to tree that replaced the tree that got rotated
Tree* tree_rotate(Tree* t, void (*update_relative_info)(Tree*));

void print_tree(Tree* t, bool reset, int row, int col, char (*get_char)(void*));
void print_info(Tree* t, void (*print_elt)(void*));
bool tree_find(Tree* t, Tree* to_find);

void* tree_get_rightmost(Tree* t);
void* tree_get_leftmost(Tree* t);

#endif
