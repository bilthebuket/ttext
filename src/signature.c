#include <stdlib.h>
#include <stdbool.h>
#include <dirent.h>
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

	while (c != ' ' && c != '\n' && c != '\0')
	{
		if (!((c >= '0' && c <= '9') || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_'))
		{
			break;
		}
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

	int store = i;
	for (; i < FUNCTION_NAME_BUFFER_SIZE; i++)
	{
		function_name_formatted[i - store] = function_name[i];
	}

	i = 0;
	c = pt_iterate(&pi);
	while (c != ';' && c != '{' && c != '}' && c != '\0' && i < FUNCTION_SIGNATURE_BUFFER_SIZE)
	{
		signature[i] = c;
		c = pt_iterate(&pi);
		i++;
	}

	LinkedList* existing = hm_get(signatures, function_name_formatted, &hash_function, &signature_equals);
	Signature* existing_signature = NULL;

	bool same = true;
	if (existing == NULL)
	{
		same = false;
	}
	else
	{
		existing_signature = (Signature*) ll_get_elt(existing, 0);
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

	if (same)
	{
		free(existing_signature->signature);
		free(function_name_formatted);
		existing_signature->signature = signature;
	}
	else
	{
		// making a copy of file name to prevent scattered pointers and double frees
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

	int parenthesis_dif = 0;
	for (size_t i = 0; i < size; i++)
	{
		if (buf[i] == '{')
		{
			if (parenthesis_dif == 0)
			{
				update_signatures(map, pt, file, i);
			}
			parenthesis_dif++;
		}
		else if (buf[i] == '}')
		{
			parenthesis_dif--;
		}
	}
}

static void initialize_signatures_helper(HashMap* map, const char* directory)
{
	DIR* dir = opendir(directory);
	if (dir == NULL)
	{
		return;
	}

	struct dirent* entry;
	while ((entry = readdir(dir)) != NULL)
	{
		const char* name = entry->d_name;
		int len = 0;
		for (; name[len] != '\0'; len++) {}
		if (len > 0 && name[len] == 'c' && name[len - 1] == '.')
		{
			add_signatures(map, name);
		}
		else
		{
			initialize_signatures_helper(map, name);
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
	return NULL;
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
