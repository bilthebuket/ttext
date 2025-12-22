#ifndef LL_H
#define LL_H

typedef struct Node
{
	void* elt;
	Node* prev;
	Node* next;
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
Node* get(LL* lst, int index);
void add(LL* lst, void* elt, int index);
void* remove(LL* lst, int index);

static Node* helper(LL* lst, int index);

#endif
