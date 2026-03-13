#ifndef LINKED_LIST_H 
#define LINKED_LIST_H 

typedef struct Node
{
	void* elt;
	struct Node* prev;
	struct Node* next;
} Node;

typedef struct LinkedList
{
	int size;
	int recent_index;
	Node* first;
	Node* last;
	Node* recent;
} LinkedList;

LinkedList* ll_create(void);
Node* ll_get_node(LinkedList* lst, int index);
void* ll_get_elt(LinkedList* lst, int index);
void ll_insert(LinkedList* lst, void* elt, int index);
void* ll_rm(LinkedList* lst, int index);
void ll_free(LinkedList* lst);

#endif
