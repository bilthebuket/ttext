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

#define FUNCTION_NAME_BUFFER_SIZE 100
#define FUNCTION_SIGNATURE_BUFFER_SIZE 200

bool signature_equals(void* v1, void* v2)
{
	Signature* s1 = (Signature*) v1;
	Signature* s2 = (Signature*) v2;

	int i = 0;
	for (; s1->file_name[i] != '\0' && s2->file_name[i] != '\0' && s1->file_name[i] == s2->file_name[i]; i++) {}
	return s1->file_name[i] == '\0' && s2->file_name[i] == '\0';
}

bool function_name_equals(void* v1, void* v2)
{
	char* name1 = (char*) v1;
	char* name2 = (char*) v2;

	int i = 0;
	for (; name1[i] != '\0' && name2[i] != '\0' && name1[i] == name2[i]; i++) {}
	return name1[i] == '\0' && name2[i] == '\0';
}

void update_signatures(HashMap* signatures, PieceTable* pt, const char* file_name, int index)
{
	if (signatures == NULL || pt == NULL)
	{
		return;
	}

	PieceIterator pi;
	if (!pt_iterator_init(pt, &pi, index))
	{
		return;
	}

	char c = pt_iterate(&pi);
	int i = index;
	for (; c != ';' && c != '}' && c != '{' && c != '\0'; c = pt_iterate(&pi), i++) {}

	if (!pt_iterator_init(pt, &pi, i - 1))
	{
		return;
	}

	c = pt_iterate_backwards(&pi);
	c = pt_iterate_backwards(&pi);
	while (1)
	{
		if (c == '\n' || c == ' ')
		{
			c = pt_iterate_backwards(&pi);
			continue;
		}
		if (c == ')')
		{
			break;
		}
		return;
	}
	c = pt_iterate_backwards(&pi);
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
			return;
		}
		c = pt_iterate_backwards(&pi);
		if (parenthesis_balance == 0)
		{
			break;
		}
	}

	for (; c == ' ' || c == '\n'; c = pt_iterate_backwards(&pi)) {}

	char function_name[FUNCTION_NAME_BUFFER_SIZE];
	function_name[FUNCTION_NAME_BUFFER_SIZE - 1] = '\0';
	i = FUNCTION_NAME_BUFFER_SIZE - 2;
	for (; i >= 0 && ((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'); i--, c = pt_iterate_backwards(&pi))
	{
		function_name[i] = c;
	}

	if (c != ' ' && c != '\n')
	{
		return;
	}

	if (is_control_word(&function_name[i + 1]))
	{
		return;
	}

	c = pt_iterate_backwards(&pi);
	for (; c == ' ' || c == '\n'; c = pt_iterate_backwards(&pi)) {}

	while (c != ' ' && c != '\n' && c != '\0')
	{
		if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'))
		{
			return;
		}
		c = pt_iterate_backwards(&pi);
	}

	c = pt_iterate_backwards(&pi);
	for (; c == ' ' || c == '\n'; c = pt_iterate_backwards(&pi)) {}

	while (c == ' ' || c == '\n' || c == '\0' || (c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_')
	{
		c = pt_iterate_backwards(&pi);
	}

	if (c == '\0')
	{
		if (!pt_iterator_init(pt, &pi, 0))
		{
			return;
		}
	}
	else
	{
		// we are flipping from backwards to forwards
		// first iteratation we are looking at the char to the left of the terminating character (the ';', '}', '{', etc; whatever broke the while loop)
		// second iteration we are looking at the terminating character
		// third iteration we are looking at the first character thats actually part of the signature
		c = pt_iterate(&pi);
		c = pt_iterate(&pi);
		c = pt_iterate(&pi);
	}

	char* function_name_formatted = malloc(sizeof(char) * FUNCTION_NAME_BUFFER_SIZE);
	if (function_name_formatted == NULL)
	{
		return;
	}
	char* signature = malloc(sizeof(char) * FUNCTION_SIGNATURE_BUFFER_SIZE);
	if (signature == NULL)
	{
		free(function_name_formatted);
		return;
	}

	i++;
	int store = i;
	for (; i < FUNCTION_NAME_BUFFER_SIZE; i++)
	{
		function_name_formatted[i - store] = function_name[i];
	}

	while (c == ' ' || c == '\n')
	{
		c = pt_iterate(&pi);
	}
	i = 0;
	while (c != ';' && c != '{' && c != '}' && c != '\0' && i < FUNCTION_SIGNATURE_BUFFER_SIZE)
	{
		signature[i] = c;
		c = pt_iterate(&pi);
		i++;
	}

	LinkedList* existing = hm_get(signatures, function_name_formatted, &hash_function, &function_name_equals);
	Signature* existing_signature = NULL;

	bool same = true;
	if (existing == NULL)
	{
		same = false;
	}
	else
	{
		existing_signature = (Signature*) ll_get_elt(existing, 0);
		if (existing_signature == NULL)
		{
			same = false;
		}
		else
		{
			i = 0;
			for (; file_name[i] != '\0' && existing_signature->file_name[i] != '\0'; i++)
			{
				if (file_name[i] != existing_signature->file_name[i])
				{
					same = false;
					break;
				}
			}
			if (existing_signature->file_name[i] != '\0')
			{
				same = false;
			}
		}
	}

	if (same)
	{
		free(existing_signature->signature);
		free(function_name_formatted);
		existing_signature->signature = signature;
	}
	else
	{
		// making a copy of file name to prevent scattered references and double frees
		int len = 0;
		for (; file_name[len] != '\0'; len++) {}
		len++;

		char* new_file_name = malloc(sizeof(char) * len);
		if (new_file_name == NULL)
		{
			free(signature);
			free(function_name_formatted);
			return;
		}

		i = 0;
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
			free(function_name_formatted);
			return;
		}

		hm_insert(signatures, function_name_formatted, new, &hash_function);
	}
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

	PieceTable* pt = pt_create(buf, size);
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
	if (r != NULL)
	{
		initialize_signatures_helper(r, ".");
	}
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
