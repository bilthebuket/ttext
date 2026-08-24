#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
#include <string.h>
#include "signature.h"
#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"
#include "global.h"
#include "hash_map.h"
#include "hash_map.h"

bool signature_equals_loose(void* v1, void* v2)
{
	Signature* s1 = (Signature*) v1;
	Signature* s2 = (Signature*) v2;

	if (v1 == NULL || v2 == NULL)
	{
		return true;
	}

	if (s1->file_name != NULL && s2->file_name != NULL && strcmp(s1->file_name, s2->file_name))
	{
		return false;
	}
	if (s1->signature != NULL && s2->signature != NULL && strcmp(s1->signature, s2->signature))
	{
		return false;
	}
	return true;
}

bool signature_equals_strict(void* v1, void* v2)
{
	Signature* s1 = (Signature*) v1;
	Signature* s2 = (Signature*) v2;

	if (s1 == NULL && s2 == NULL)
	{
		return true;
	}
	if ((s1 == NULL && s2 != NULL) || (s1 != NULL && s2 == NULL))
	{
		return false;
	}
	if ((s1->file_name == NULL && s2->file_name != NULL) || (s1->file_name != NULL && s2->file_name == NULL))
	{
		return false;
	}
	if (s1->file_name != NULL && s2->file_name != NULL && strcmp(s1->file_name, s2->file_name))
	{
		return false;
	}
	if ((s1->signature == NULL && s2->signature != NULL) || (s1->signature != NULL && s2->signature == NULL))
	{
		return false;
	}
	if (s1->signature != NULL && s2->signature != NULL && strcmp(s1->signature, s2->signature))
	{
		return false;
	}
	return true;
}

bool function_name_equals(void* v1, void* v2)
{
	char* name1 = (char*) v1;
	char* name2 = (char*) v2;

	if (name1 == NULL && name2 == NULL)
	{
		return true;
	}
	if (name1 == NULL || name2 == NULL)
	{
		return false;
	}
	return !strcmp(name1, name2);
}

static int iterate_to_signature_start(PieceTable* pt, PieceIterator* pi, int index, char** store_signature_and_function_name)
{
	if (!pt_iterator_init(pt, pi, index))
	{
		return -1;
	}

	int signature_start_index;
	int signature_end_index;
	int name_start_index;
	int name_end_index;

	char c = pt_iterate(pi);
	int i = index;
	for (; c != ';' && c != '}' && c != '{' && c != '\0'; c = pt_iterate(pi), i++) {}

	if (c == '\0')
	{
		if (!pt_iterator_init(pt, pi, i - 1))
		{
			return -1;
		}
		index = i - 1;
	}
	else
	{
		c = pt_iterate_backwards(pi);
		c = pt_iterate_backwards(pi);
		c = pt_iterate_backwards(pi);
		index = i - 1;
		if (c == '\0')
		{
			if (!pt_iterator_init(pt, pi, index))
			{
				return -1;
			}
			c = pt_iterate_backwards(pi);
		}
	}
	while (1)
	{
		if (c == '\n' || c == ' ')
		{
			c = pt_iterate_backwards(pi);
			index--;
			continue;
		}
		if (c == ')')
		{
			break;
		}
		return -1;
	}
	signature_end_index = index;
	c = pt_iterate_backwards(pi);
	index--;
	int parenthesis_balance = 1;
	while (1)
	{
		if (c == ')')
		{
			parenthesis_balance++;
		}
		else if (c == '(')
		{
			parenthesis_balance--;
		}
		else if (c == '}' || c == '{' || c == ';')
		{
			return -1;
		}
		c = pt_iterate_backwards(pi);
		index--;
		if (parenthesis_balance == 0 || c == '\0')
		{
			break;
		}
	}

	for (; c == ' ' || c == '\n'; c = pt_iterate_backwards(pi), index--) {}

	name_end_index = index;
	for (; is_valid_name_character(c); c = pt_iterate_backwards(pi), index--) {}
	name_start_index = index + 1;

	if (c != ' ' && c != '\n')
	{
		return -1;
	}

	c = pt_iterate_backwards(pi);
	index--;
	for (; c == ' ' || c == '\n'; c = pt_iterate_backwards(pi), index--) {}

	while (c != ' ' && c != '\n' && c != '\0')
	{
		if (!is_valid_name_character(c))
		{
			return -1;
		}
		c = pt_iterate_backwards(pi);
		index--;
	}

	c = pt_iterate_backwards(pi);
	index--;
	for (; c == ' ' || c == '\n'; c = pt_iterate_backwards(pi), index--) {}

	while (c == ' ' || c == '\n' || is_valid_name_character(c))
	{
		c = pt_iterate_backwards(pi);
		index--;
	}

	if (c == '\0')
	{
		if (!pt_iterator_init(pt, pi, 0))
		{
			return -1;
		}
		index = 0;

		char c2 = pt_get(pt, 0);
		if (c2 == ' ' || c2 == '\n')
		{
			c = pt_iterate(pi);
		}
	}
	else
	{
		c = pt_iterate(pi);
		c = pt_iterate(pi);
		c = pt_iterate(pi);
		index++;
	}

	if (c != '\0')
	{
		while (c == ' ' || c == '\n')
		{
			c = pt_iterate(pi);
			index++;
		}

		c = pt_iterate_backwards(pi);
	}

	signature_start_index = index;

	if (store_signature_and_function_name != NULL)
	{
		PieceIterator pi2;

		int signature_len = signature_end_index - signature_start_index + 1;
		char* signature  = malloc(sizeof(char) * (signature_len + 1));
		if (signature == NULL)
		{
			return -1;
		}

		int function_name_len = name_end_index - name_start_index + 1;
		char* function_name = malloc(sizeof(char) * (function_name_len + 1));
		if (function_name == NULL)
		{
			free(signature);
			return -1;
		}

		if (pt_iterator_init(pt, &pi2, signature_start_index))
		{
			c = pt_iterate(&pi2);
			i = 0;
			for (i = 0; i < signature_len; i++, c = pt_iterate(&pi2))
			{
				signature[i] = c;
			}
			signature[i] = '\0';
		}
		if (pt_iterator_init(pt, &pi2, name_start_index))
		{
			c = pt_iterate(&pi2);
			i = 0;
			for (i = 0; i < function_name_len; i++, c = pt_iterate(&pi2))
			{
				function_name[i] = c;
			}
			function_name[i] = '\0';

			if (is_control_word(function_name))
			{
				free(signature);
				free(function_name);
				return -1;
			}
		}

		store_signature_and_function_name[SIGNATURE_FUNCTIONS_SIGNATURE] = signature;
		store_signature_and_function_name[SIGNATURE_FUNCTIONS_FUNCTION_NAME] = function_name;
	}

	return index;
}

void update_signatures(HashMap* signatures, PieceTable* pt, const char* file_name, int index)
{
	if (signatures == NULL || pt == NULL || file_name == NULL)
	{
		return;
	}

	PieceIterator pi;
	char* function_name_and_signature[SIGNATURE_FUNCTIONS_NUM_RETURN_VALS];

	if (iterate_to_signature_start(pt, &pi, index, function_name_and_signature) < 0)
	{
		return;
	}

	char* signature = function_name_and_signature[SIGNATURE_FUNCTIONS_SIGNATURE];
	char* function_name = function_name_and_signature[SIGNATURE_FUNCTIONS_FUNCTION_NAME];

	// making a copy of file name to prevent scattered references and double frees
	int len = 0;
	for (; file_name[len] != '\0'; len++) {}
	len++;

	char* new_file_name = malloc(sizeof(char) * len);
	if (new_file_name == NULL)
	{
		free(signature);
		free(function_name);
		return;
	}

	int i = 0;
	for (; file_name[i] != '\0'; i++)
	{
		new_file_name[i] = file_name[i];
	}
	new_file_name[i] = '\0';

	Signature* new = signature_create(signature, new_file_name);
	if (new == NULL)
	{
		free(signature);
		free(new_file_name);
		free(function_name);
		return;
	}

	hm_insert(signatures, function_name, new, &hash_function);
}

static void add_signatures(HashMap* map, const char* file)
{
	FILE* f = fopen(file, "r");
	if (f == NULL)
	{
		return;
	}

	fseek(f, 0, SEEK_END);
	size_t size = ftell(f);
	rewind(f);
	char* buf = malloc(sizeof(char) * size);
	if (buf == NULL)
	{
		return;
	}
	size = fread(buf, sizeof(char), size, f);

	PieceTable* pt = pt_create(buf, size, false);
	if (pt == NULL)
	{
		return;
	}

	PieceIterator pi;
	if (!pt_iterator_init(pt, &pi, 0))
	{
		return;
	}

	char c = pt_iterate(&pi);
	int brace_dif = 0;
	for (int i = 0; c != '\0'; i++, c = pt_iterate(&pi))
	{
		if (c == '{')
		{
			if (brace_dif == 0)
			{
				update_signatures(map, pt, file, i);
			}
			brace_dif++;
		}
		else if (c == '}')
		{
			brace_dif--;
		}
	}

	pt_free(pt);
}

static char* concat_directory_path(const char* parent, const char* file)
{
	int len1 = 0;
	int len2 = 0;
	for (; parent[len1] != '\0'; len1++) {}
	for (; file[len2] != '\0'; len2++) {}
	// 2 = one for '\0' plus one for '/'
	char* r = malloc(sizeof(char) * (len1 + len2 + 2));
	if (r != NULL)
	{
		for (int i = 0; i < len1; i++)
		{
			r[i] = parent[i];
		}
		r[len1] = '/';
		for (int i = 0; i < len2; i++)
		{
			r[i + len1 + 1] = file[i];
		}
		r[len1 + len2 + 1] = '\0';
	}

	return r;
}

static void initialize_signatures_helper(HashMap* map, const char* directory)
{
	if (directory == NULL)
	{
		return;
	}

	DIR* dir = opendir(directory);
	if (dir == NULL)
	{
		return;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		const char* name = entry->d_name;
		char* path = concat_directory_path(directory, name);
		if (path != NULL)
		{
			int len = 0;
			for (; path[len] != '\0'; len++) {}
			if (len > 0 && path[len - 1] == 'c' && path[len - 2] == '.')
			{
				add_signatures(map, path);
			}
			else if (strcmp(name, ".") && strcmp(name, ".."))
			{
				initialize_signatures_helper(map, path);
			}
			free(path);
		}
	}

	closedir(dir);
}

HashMap* initialize_signatures(void)
{
	HashMap* r = hm_create();
	if (r == NULL)
	{
		free(r);
		return NULL;
	}
	initialize_signatures_helper(r, ".");
	return r;
}

Signature* signature_create(char* signature, char* file_name)
{
	Signature* r = malloc(sizeof(Signature));
	if (r != NULL)
	{
		r->signature = signature;
		r->file_name = file_name;
	}
	return r;
}

void signature_free(void* v)
{
	if (v == NULL)
	{
		return;
	}
	Signature* s = (Signature*) v;

	if (s->signature != NULL)
	{
		free(s->signature);
	}
	if (s->file_name != NULL)
	{
		free(s->file_name);
	}
	free(s);
}

void print_all_signatures(HashMap* signatures, FILE* f)
{
	if (signatures == NULL || f == NULL)
	{
		return;
	}

	for (int i = 0; i < signatures->arr_size; i++)
	{
		LinkedList* lst = (LinkedList*) signatures->arr[i];
		if (lst == NULL)
		{
			continue;
		}
		for (int j = 0; j < lst->size; j++)
		{
			HashMapElt* elt = (HashMapElt*) ll_get_elt(lst, j);
			Signature* s = (Signature*) elt->value;
			char* key = (char*) elt->key;
			if (s != NULL && s->file_name != NULL && s->signature != NULL)
			{
				fprintf(f, "index %d | function name %s | signature %s | file_name %s\n", i, key, s->signature, s->file_name);
				fflush(f);
			}
		}
	}
}

char** in_signature_huh(HashMap* signatures, PieceTable* pt, int index, int* store_start_index)
{
	if (signatures == NULL || pt == NULL || index < 0)
	{
		return false;
	}

	char** r = malloc(sizeof(char*) * SIGNATURE_FUNCTIONS_NUM_RETURN_VALS);
	if (r == NULL)
	{
		return NULL;
	}

	PieceIterator pi;
	int start_index = iterate_to_signature_start(pt, &pi, index, r);
	if (start_index < 0)
	{
		free(r);
		return NULL;
	}

	char c = pt_iterate(&pi);
	while (c != '{' && c != '}' && c != ';')
	{
		if (index == start_index)
		{
			if (store_start_index != NULL)
			{
				*store_start_index = start_index;
			}
			return r;
		}
		c = pt_iterate(&pi);
		start_index++;
	}

	free(r[SIGNATURE_FUNCTIONS_FUNCTION_NAME]);
	free(r[SIGNATURE_FUNCTIONS_SIGNATURE]);
	free(r);
	return NULL;
}

/*
static bool file_name_equals(void* v, bool compare)
{
	static char* compare_to = NULL;
	if (!compare)
	{
		compare_to = (char*) v;
		return false;
	}
	else if (compare_to != NULL)
	{
		return !strcmp(((Signature*) v)->file_name, compare_to);
	}
	else
	{
		return false;
	}
}
*/

// always reset compare_to to NULL when your done doing comparison to avoid hard to follow state
static bool signature_equals2(void* v, bool compare)
{
	static Signature* compare_to = NULL;
	if (!compare)
	{
		compare_to = (Signature*) v;
		return false;
	}
	else if (compare_to != NULL)
	{
		return signature_equals_loose(v, compare_to);
	}
	else
	{
		return false;
	}
}

void remove_signature(HashMap* signatures, char* function_name, char* signature, char* file_name)
{
	if (signatures == NULL || function_name == NULL)
	{
		return;
	}

	Signature s;
	s.file_name = file_name;
	s.signature = signature;

	signature_equals2(&s, false);

	Signature* removed = hm_rm_one(signatures, function_name, &hash_function, &function_name_equals, &signature_equals2, &free);
	signature_free(removed);
}

void update_signatures_on_boundary(HashMap* signatures, PieceTable* pt, char* file_name, int start_index, int end_index)
{
	if (signatures == NULL || pt == NULL || file_name == NULL || start_index < 0 || end_index < start_index)
	{
		return;
	}

	PieceIterator pi;
	if (!pt_iterator_init(pt, &pi, start_index))
	{
		return;
	}

	char c = pt_iterate(&pi);
	int brace_dif = 0;
	for (int i = start_index; c != '\0' && i <= end_index; i++, c = pt_iterate(&pi))
	{
		if (c == '{')
		{
			if (brace_dif == 0)
			{
				update_signatures(signatures, pt, file_name, i);
			}
			brace_dif++;
		}
		else if (c == '}')
		{
			brace_dif--;
		}
	}
}

void remove_signatures_on_boundary(HashMap* signatures, PieceTable* pt, char* file_name, int start_index, int end_index)
{
	if (signatures == NULL || pt == NULL || file_name == NULL || start_index < 0 || end_index < start_index)
	{
		return;
	}

	PieceIterator pi;
	if (!pt_iterator_init(pt, &pi, start_index))
	{
		return;
	}

	char c = pt_iterate(&pi);
	int brace_dif = 0;
	for (int i = start_index; c != '\0' && i <= end_index; i++, c = pt_iterate(&pi))
	{
		if (c == '{')
		{
			if (brace_dif == 0)
			{
				char** vals = in_signature_huh(signatures, pt, i - 1, NULL);
				if (vals != NULL)
				{
					remove_signature(signatures, vals[SIGNATURE_FUNCTIONS_FUNCTION_NAME], vals[SIGNATURE_FUNCTIONS_SIGNATURE], file_name);
					free(vals[SIGNATURE_FUNCTIONS_FUNCTION_NAME]);
					free(vals[SIGNATURE_FUNCTIONS_SIGNATURE]);
					free(vals);
				}
			}
			brace_dif++;
		}
		else if (c == '}')
		{
			brace_dif--;
		}
	}
}

void su_prepare(HashMap* signatures, PieceTable* pt, SignatureUpdate* su, char* file_name, int index)
{
	if (signatures == NULL || pt == NULL || su == NULL || file_name == NULL || index < 0)
	{
		return;
	}

	int start_index;
	char** vals = in_signature_huh(signatures, pt, index, &start_index);
	if (vals == NULL)
	{
		PieceIterator pi;
		if (!pt_iterator_init(pt, &pi, index))
		{
			su->start_index = index;
			su->end_index = index;
			return;
		}

		int end_index = index;
		char c = pt_iterate(&pi);
		for (; c != '{' && c != '}' && c != ';'; c = pt_iterate(&pi), end_index++) {}

		su->start_index = index;
		su->end_index = end_index;
	}
	else
	{
		PieceIterator pi;
		if (!pt_iterator_init(pt, &pi, index))
		{
			su->start_index = index;
			su->end_index = index;
			return;
		}

		int end_index = index;
		char c = pt_iterate(&pi);
		for (; c != '{' && c != '}' && c != ';'; c = pt_iterate(&pi), end_index++) {}

		su->start_index = start_index;
		su->end_index = end_index;

		remove_signature(signatures, vals[SIGNATURE_FUNCTIONS_FUNCTION_NAME], vals[SIGNATURE_FUNCTIONS_SIGNATURE], file_name);
		free(vals[SIGNATURE_FUNCTIONS_FUNCTION_NAME]);
		free(vals[SIGNATURE_FUNCTIONS_SIGNATURE]);
		free(vals);
	}
}

void su_handle_insertion(HashMap* signatures, PieceTable* pt, char* file_name, SignatureUpdate* su, int index)
{
	if (su == NULL)
	{
		return;
	}

	int start_index;
	char** vals = in_signature_huh(signatures, pt, index, &start_index);
	if (vals != NULL)
	{
		remove_signature(signatures, vals[SIGNATURE_FUNCTIONS_FUNCTION_NAME], vals[SIGNATURE_FUNCTIONS_SIGNATURE], file_name);
		free(vals[SIGNATURE_FUNCTIONS_FUNCTION_NAME]);
		free(vals[SIGNATURE_FUNCTIONS_SIGNATURE]);
		free(vals);
	}
	
	su->end_index++;
}

void su_handle_deletion(HashMap* signatures, PieceTable* pt, char* file_name, SignatureUpdate* su, int index)
{
	if (su == NULL || index < 0)
	{
		return;
	}

	int start_index;
	char** vals = in_signature_huh(signatures, pt, index, &start_index);
	if (vals != NULL)
	{
		remove_signature(signatures, vals[SIGNATURE_FUNCTIONS_FUNCTION_NAME], vals[SIGNATURE_FUNCTIONS_SIGNATURE], file_name);
		free(vals[SIGNATURE_FUNCTIONS_FUNCTION_NAME]);
		free(vals[SIGNATURE_FUNCTIONS_SIGNATURE]);
		free(vals);
	}

	if (su->start_index == index)
	{
		su->start_index--;
	}

	su->end_index--;
}

void su_handle_multiple_rm(HashMap* signatures, PieceTable* pt, char* file_name, SignatureUpdate* su, int num_deleted, int index)
{
	if (su == NULL || index < 0 || num_deleted <= 0)
	{
		return;
	}

	int start_index = index;
	while (start_index > index - num_deleted)
	{
		char** vals = in_signature_huh(signatures, pt, start_index, &start_index);
		if (vals != NULL)
		{
			remove_signature(signatures, vals[SIGNATURE_FUNCTIONS_FUNCTION_NAME], vals[SIGNATURE_FUNCTIONS_SIGNATURE], file_name);
			free(vals[SIGNATURE_FUNCTIONS_FUNCTION_NAME]);
			free(vals[SIGNATURE_FUNCTIONS_SIGNATURE]);
			free(vals);
		}
		else
		{
			start_index--;
		}
	}

	if (su->start_index >= index - num_deleted + 1)
	{
		su->start_index = index - num_deleted + 1;
	}

	su->end_index -= num_deleted;
}

void su_execute(HashMap* signatures, PieceTable* pt, SignatureUpdate* su, char* file_name)
{
	if (su == NULL)
	{
		return;
	}

	update_signatures_on_boundary(signatures, pt, file_name, su->start_index, su->end_index);
}
