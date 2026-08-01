#ifndef HASH_MAP_H
#define HASH_MAP_H

#include <stdbool.h>
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

// elt_equals has a boolean which will be set to true when doing the comparison, and false for when we are setting up the function for comparison
// the idea is that elt_equals functions will have static variable(s) for comparison which will be initialized before hm_rm and hm_get are called
// it creates spaghetti code and hard to follow state but the alternative would be to pass key as a struct with both the key and whatever we need for the deep equality check, 
// which would be very annoying for normal use that does not need deep equality.
// another alternative would be to not do deep equality at all which creates a mess of removing and immediatly re adding elts
LinkedList* hm_rm(HashMap* map, void* key, int (*hash)(void*, int), bool (*key_equals)(void*, void*), bool (*elt_equals)(void*, bool), void (*key_free)(void*));
// same as hm_rm except it removes the first instance that meets the criteria from removal and returns instead of removing the rest
void* hm_rm_one(HashMap* map, void* key, int (*hash)(void*, int), bool (*key_equals)(void*, void*), bool (*elt_equals)(void*, bool), void (*key_free)(void*));
LinkedList* hm_get(HashMap* map, void* key, int (*hash)(void*, int), bool (*key_equals)(void*, void*), bool (*elt_equals)(void*, bool));

// returns the linked list for a key that the hashmap uses internally
// use with extremem caution, the linked list should be read only (dont add or remove linked list nodes)
LinkedList* hm_get_dangerous(HashMap* map, void* key, int (*hash)(void*, int));
HashMap* hm_create(void);
void hm_free(HashMap* map, void (*key_free)(void*), void (*value_free)(void*));

HashMapElt* hm_elt_create(void* key, void* value);

#endif
