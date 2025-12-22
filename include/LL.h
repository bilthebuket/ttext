#ifndef LL_H
#define LL_H

typedef struct Node
{
	void* elt;
	struct Node* prev;
	struct Node* next;
} Node;

typedef struct LL
{
	int size;
	int recent_index;
	Node* first;
	Node* last;
	Node* recent;
} LL;

LL* make_list(void);
Node* get_node(LL* lst, int index);
void* get_elt(LL* lst, int index);
void add(LL* lst, void* elt, int index);
void* rm(LL* lst, int index);
void free_list(LL* lst);

static Node* helper(LL* lst, int index);

#endif
