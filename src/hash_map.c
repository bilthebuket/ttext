#include <stdlib.h>
#include "hash_map.h"
#include "linked_list.h"

HashMap* hm_create(void)
{
	HashMap* r = malloc(sizeof(HashMap));
	if (r != NULL)
	{
		r->arr = malloc(sizeof(LinkedList*) * INITIAL_SIZE);
		if (r->arr == NULL)
		{
			free(r);
			return NULL;
		}
		r->arr_size = INITIAL_SIZE;
		for (int i = 0; i < r->arr_size; i++)
		{
			r->arr[i] = NULL;
		}
		r->num_items = 0;
	}
	return r;
}

static void resize(HashMap* map, int (*hash)(void*, int))
{
	int new_arr_size = map->arr_size * 2;
	LinkedList** new_arr = malloc(sizeof(LinkedList*) * new_arr_size);

	for (int i = 0; i < new_arr_size; i++)
	{
		new_arr[i] = NULL;
	}

	LinkedList** old_arr = map->arr;
	int old_arr_size = map->arr_size;
	map->arr = new_arr;
	map->arr_size = new_arr_size;
	map->num_items = 0;
	for (int i = 0; i < old_arr_size; i++)
	{
		LinkedList* lst = old_arr[i];
		if (lst != NULL)
		{
			HashMapElt* elt = (HashMapElt*) ll_rm(lst, 0);
			while (elt != NULL)
			{
				hm_insert(map, elt->key, elt->value, hash);
				free(elt);
				elt = (HashMapElt*) ll_rm(lst, 0);
			}
			ll_free(lst);
		}
	}
	free(old_arr);
}

HashMapElt* hm_elt_create(void* key, void* value)
{
	HashMapElt* r = malloc(sizeof(HashMapElt));
	if (r != NULL)
	{
		r->key = key;
		r->value = value;
	}
	return r;
}

void hm_insert(HashMap* map, void* key, void* value, int (*hash)(void*, int))
{
	if (map == NULL || hash == NULL)
	{
		return;
	}

	int index = (*hash)(key, map->arr_size);
	if (index < 0 || index >= map->arr_size)
	{
		return;
	}

	LinkedList* lst = map->arr[index];
	if (lst == NULL)
	{
		map->arr[index] = ll_create();
		if (map->arr[index] == NULL)
		{
			return;
		}
		lst = map->arr[index];
	}

	HashMapElt* elt = hm_elt_create(key, value);
	if (elt != NULL)
	{
		ll_insert(lst, elt, 0);
		map->num_items++;
	}
	else
	{
		return;
	}

	if ((double) map->num_items > ((double) map->arr_size) * LOAD_FACTOR)
	{
		resize(map, hash);
	}
}

LinkedList* hm_rm(HashMap* map, void* key, int (*hash)(void*, int), bool (*key_equals)(void*, void*), bool (*elt_equals)(void*, bool), void (*key_free)(void*))
{
	if (map == NULL || hash == NULL)
	{
		return NULL;
	}

	int index = (*hash)(key, map->arr_size);
	if (index < 0 || index >= map->arr_size)
	{
		return NULL;
	}

	LinkedList* lst = map->arr[index];
	if (lst == NULL)
	{
		return NULL;
	}

	LinkedList* r = ll_create();
	if (r != NULL)
	{
		int i = 0;
		while (1)
		{
			HashMapElt* elt = (HashMapElt*) ll_get_elt(lst, i);
			if (elt == NULL)
			{
				break;
			}

			bool are_keys_equal = true;
			if (key_equals != NULL)
			{
				are_keys_equal = (*key_equals)(key, elt->key);
			}
			if (elt_equals != NULL)
			{
				are_keys_equal &&= (*elt_equals)(elt->value, true);
			}

			if (are_keys_equal)
			{
				ll_insert(r, elt->value, 0);
				ll_rm(lst, i);
				if (key_free != NULL)
				{
					(*key_free)(elt->key);
				}
				free(elt);
			}
			else
			{
				i++;
			}
		}
	}

	return r;
}

LinkedList* hm_get(HashMap* map, void* key, int (*hash)(void*, int), bool (*key_equals)(void*, void*), bool (*elt_equals)(void*, bool))
{
	if (map == NULL || hash == NULL)
	{
		return NULL;
	}

	int index = (*hash)(key, map->arr_size);
	if (index < 0 || index >= map->arr_size)
	{
		return NULL;
	}

	LinkedList* lst = map->arr[index];
	if (lst == NULL)
	{
		return NULL;
	}

	LinkedList* r = ll_create();
	if (r != NULL)
	{
		int i = 0;
		while (1)
		{
			HashMapElt* elt = ll_get_elt(lst, i);
			if (elt == NULL)
			{
				break;
			}

			bool are_keys_equal = true;
			if (key_equals != NULL)
			{
				are_keys_equal = (*key_equals)(key, elt->key);
			}
			if (elt_equals != NULL)
			{
				are_keys_equals &&= (*elt_equals)(elt->value, true);
			}

			if (are_keys_equal)
			{
				ll_insert(r, elt->value, 0);
			}

			i++;
		}
	}

	return r;
}

LinkedList* hm_get_dangerous(HashMap* map, void* key, int (*hash)(void*, int))
{
	if (map == NULL || hash == NULL)
	{
		return NULL;
	}

	int index = (*hash)(key, map->arr_size);
	if (index < 0 || index >= map->arr_size)
	{
		return NULL;
	}

	return map->arr[index];
}

void hm_free(HashMap* map, void (*key_free)(void*), void (*value_free)(void*))
{
	if (map == NULL)
	{
		return;
	}

	for (int i = 0; i < map->arr_size; i++)
	{
		LinkedList* lst = map->arr[i];

		if (lst != NULL)
		{
			HashMapElt* elt = (HashMapElt*) ll_rm(lst, 0);
			while (elt != NULL)
			{
				if (key_free != NULL)
				{
					(*key_free)(elt->key);
				}
				if (value_free != NULL)
				{
					(*value_free)(elt->value);
				}
				free(elt);
				elt = (HashMapElt*) ll_rm(lst, 0);
			}
			ll_free(lst);
		}
	}

	free(map->arr);
	free(map);
}
