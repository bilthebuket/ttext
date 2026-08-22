#ifndef DYNAMIC_ARRAY_H
#define DYNAMIC_ARRAY_H

typedef struct DynamicArray
{
	char* arr;
	int arr_size;
	int len;
}

DynamicArray* da_create(int starting_size);
char da_get(DyanmicArray* da, int index);
void da_insert(DynamicArray* da, char c, int index);
void da_rm(DynamicArray* da, int index);
void da_free(DynamicArray* da);
char* da_convert_to_fixed(DynamicArray* da);
