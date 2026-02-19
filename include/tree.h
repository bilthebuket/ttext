#ifndef TREE_H
#define TREE_H

typedef struct Tree
{
	void* elt;
	struct Tree* left;
	struct Tree* right;
	struct Tree* prev;
	int height;
} Tree;

Tree* tree_helper(Tree* t, void* elt, int (*cmp)(void*, void*));
Tree* tree_create(void* elt);
Tree* tree_add_elt(Tree* t, void* elt, int (*cmp)(void*, void*));
Tree* tree_add_tree(Tree* t, Tree* to_add, int (*cmp)(void*, void*));
Tree* tree_rm(Tree* t, void* elt, int (*cmp)(void*, void*), void (*free_node)(void*));
void* tree_get(Tree* t, void* elt, int (*cmp)(void*, void*));
void tree_free(Tree* t, void (*free_node)(void*));
Tree* tree_balance(Tree* t, int (*cmp)(void*, void*));

// only updates the height of t, not any of the nodes connected to it
void update_height(Tree* t);

// returns pointer to tree that replaced the tree that got rotated
Tree* tree_rotate(Tree* t, int (*cmp)(void*, void*));

#endif
