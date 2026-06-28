#ifndef HASH_MAP_H
#define HASH_MAP_H

#include "linked_list.h"

#define INITIAL_SIZE 128
#define LOAD_FACTOR .5

typedef struct HashMap
{
	LinkedList** arr;
	int arr_size;
	int num_items;
} HashMap;

typedef struct HashMapElt
{
	void* key;
	void* value;
} HashMapElt;

void hm_insert(HashMap* map, void* key, void* value, int (*hash)(void*, int));
LinkedList* hm_rm(HashMap* map, void* key, int (*hash)(void*, int), bool (*equals)(void*, void*), void (*key_free)(void*));
LinkedList* hm_get(HashMap* map, void* key, int (*hash)(void*, int), bool (*equals)(void*, void*));

// returns the linked list for a key that the hashmap uses internally
// use with extremem caution, the linked list should be read only (dont add or remove linked list nodes)
LinkedList* hm_get_dangerous(HashMap* map, void* key, int (*hash)(void*, int));
HashMap* hm_create(void);
void hm_free(HashMap* map, void (*key_free)(void*), void (*value_free)(void*));

HashMapElt* hm_elt_create(void* key, void* value);

#endif
