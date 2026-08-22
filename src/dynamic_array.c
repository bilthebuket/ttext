#include <stdlib.h>
#include "dynamic_array.h"

DynamicArray* da_create(int starting_size)
{
	if (starting_size <= 0)
	{
		starting_size = DYNAMIC_ARRAY_DEFAULT_SIZE;
	}

	DynamicArray* r = malloc(sizeof(DynamicArray));
	if (r != NULL)
	{
		r->arr = malloc(sizeof(char) * start_size);
		if (r->arr == NULL)
		{
			free(r);
			return NULL;
		}
		r->arr_size = starting_size;
		r->len = 0;
	}
	return r;
}

char da_get(DyanmicArray* da, int index)
{
	if (da == NULL || index < 0 || index >= len)
	{
		return '\0';
	}

	return da->arr[index];
}

void da_insert(DynamicArray* da, char c, int index)
{
	if (da == NULL || index < 0 || index > da->len)
	{
		return;
	}

	if (da->len == da->arr_size)
	{
		char* new_arr = realloc(da->arr_size, sizeof(char) * da->arr_size * 2);
		if (new_arr == NULL)
		{
			return;
		}
		else
		{
			da->arr = new_arr;
			da->arr_size *= 2;
		}
	}

	for (int i = da->len - 1; i >= index; i--)
	{
		da->arr[i + 1] = da->arr[i];
	}

	da->arr[index] = c;
}

char da_rm(DynamicArray* da, int index)
{
	if (da == NULL || index < 0 || index >= da->len)
	{
		return '\0';
	}

	char r = da->arr[index];

	for (int i = index; i < da->len - 1; i++)
	{
		da->arr[i] = da->arr[i + 1];
	}

	return r;
}

void da_free(DynamicArray* da)
{
	if (da == NULL)
	{
		return;
	}

	free(da->arr);
	free(da);
}
