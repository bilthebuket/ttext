#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "fuzzy_find.h"
#include "global.h"
#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"
#include "io_tools.h"

#define ROWS 4
#define KEYS_PER_ROW 13
#define ARBITRARY_VALUE 50

static int proximities[NUM_CHARS][NUM_CHARS];
static char qwerty[ROWS][KEYS_PER_ROW * 2 + 1] = 
{
	"1!2@3#4$5%6^7&8*9(0)-_=+\0\0", 
	"qQwWeErRtTyYuUiIoOpP[{]}\\|",
	"aAsSdDfFgGhHjJkKlL;:\'\"\0\0\0\0",
	"zZxXcCvVbBnNmM,<.>/?\0\0\0\0\0\0"
};

void fuzzy_find_init(void)
{
	for (int i = 0; i < ROWS; i++)
	{
		for (int j = 0; j < KEYS_PER_ROW; j++)
		{
			for (int k = -1; k <= 1; k++)
			{
				for (int w = -1; w <= 1; w++)
				{
					if (i + k >= 0 && i + k < ROWS)
					{
						if (j * 2 + w + 1 >= 0 && j * 2 + w < KEYS_PER_ROW * 2)
						{
							proximities[(int) qwerty[i][j * 2 + 1]][(int) qwerty[i + k][j * 2 + 1 + w]] = 1;
							proximities[(int) qwerty[i][j * 2]][(int) qwerty[i + k][j * 2 + 1 + w]] = 1;
						}
						if (j * 2 + w >= 0 && j * 2 + w < KEYS_PER_ROW * 2)
						{
							proximities[(int) qwerty[i][j * 2 + 1]][(int) qwerty[i + k][j * 2 + w]] = 1;
							proximities[(int) qwerty[i][j * 2]][(int) qwerty[i + k][j * 2 + w]] = 1;
						}
					}
				}
			}

			proximities[(int) qwerty[i][j * 2]][(int) qwerty[i][j * 2]] = 2;
			proximities[(int) qwerty[i][j * 2 + 1]][(int) qwerty[i][j * 2 + 1]] = 2;
			proximities[(int) qwerty[i][j * 2]][(int) qwerty[i][j * 2 + 1]] = 2;
			proximities[(int) qwerty[i][j * 2 + 1]][(int) qwerty[i][j * 2]] = 2;
		}
	}
}

static void merge_sort(char** to_sort, int* sort_by, int len)
{
	if (len == 1 || len == 0)
	{
		return;
	}
	else if (len == 2)
	{
		if (sort_by[0] > sort_by[1])
		{
			char* store1 = to_sort[0];
			to_sort[0] = to_sort[1];
			to_sort[1] = store1;

			int store2 = sort_by[0];
			sort_by[0] = sort_by[1];
			sort_by[1] = store2;
		}

		return;
	}

	merge_sort(&to_sort[0], &sort_by[0], len / 2);
	merge_sort(&to_sort[len / 2], &sort_by[len / 2], len - (len / 2));

	char** to_sort_store = malloc(sizeof(char*) * len);
	if (to_sort_store == NULL)
	{
		return;
	}
	int* sort_by_store = malloc(sizeof(int) * len);
	if (sort_by_store == NULL)
	{
		free(to_sort_store);
		return;
	}

	int left_index = 0;
	int right_index = len / 2;
	for (int i = 0; i < len; i++)
	{
		if (left_index == len / 2)
		{
			to_sort_store[i] = to_sort[right_index];
			sort_by_store[i] = sort_by[right_index];
			right_index++;
		}
		else if (right_index == len)
		{
			to_sort_store[i] = to_sort[left_index];
			sort_by_store[i] = sort_by[left_index];
			left_index++;
		}
		else if (sort_by[left_index] > sort_by[right_index])
		{
			to_sort_store[i] = to_sort[right_index];
			sort_by_store[i] = sort_by[right_index];
			right_index++;
		}
		else
		{
			to_sort_store[i] = to_sort[left_index];
			sort_by_store[i] = sort_by[left_index];
			left_index++;
		}
	}

	memcpy(to_sort, to_sort_store, sizeof(char*) * len);
	memcpy(sort_by, sort_by_store, sizeof(int) * len);
	free(to_sort_store);
	free(sort_by_store);
}

void order_by_closest_match(char** to_order, int to_order_len, char* target)
{
	char target_instances[NUM_CHARS];
	for (int i = 0; i < NUM_CHARS; i++)
	{
		target_instances[i] = 0;
	}

	for (int i = 0; target[i] != '\0'; i++)
	{
		for (int j = 0; j < NUM_CHARS; j++)
		{
			target_instances[j] += proximities[(int) target[i]][j];
		}
	}

	int* residuals = calloc(to_order_len, sizeof(int));
	if (residuals == NULL)
	{
		return;
	}

	for (int i = 0; i < to_order_len; i++)
	{
		char this_instances[NUM_CHARS];
		for (int j = 0; j < NUM_CHARS; j++)
		{
			this_instances[j] = 0;
		}
		for (int j = 0; to_order[i][j] != '\0' && target[j] != '\0'; j++)
		{
			for (int k = 0; k < NUM_CHARS; k++)
			{
				this_instances[k] += proximities[(int) to_order[i][j]][k];
			}
		}

		for (int j = 0; j < NUM_CHARS; j++)
		{
			residuals[i] += abs(this_instances[j] - target_instances[j]);
		}
	}

	merge_sort(to_order, residuals, to_order_len);
	free(residuals);
}

char** find_strings_to_autocomplete(EditorState* es, int* arr_len)
{
	Tab* t = es->active_tab;
	if (t == NULL)
	{
		return NULL;
	}
	
	HashMap* found = hm_create();
	if (found == NULL)
	{
		return NULL;
	}

	LinkedList* lst = hm_get_all_keys(es->signatures);
	if (lst == NULL)
	{
		hm_free(found, NULL, NULL);
		return NULL;
	}

	for (int i = 0; i < lst->size; i++)
	{
		char* key = ll_rm(lst, 0);
		if (key != NULL)
		{
			int len = strlen(key) + 1;
			char* dupe = malloc(sizeof(char) * len);
			if (dupe != NULL)
			{
				strcpy(dupe, key);
				ll_insert(lst, dupe, lst->size);
				hm_insert(found, dupe, NULL, &hash_function);
			}
		}
	}

	PieceIterator pi;
	PieceIterator ci;
	if (pt_iterator_init(t->pt, &pi, 0) && ci_iterator_init(t->pt, &ci, 0))
	{
		char c = pt_iterate(&pi);
		int color = ci_iterate(&ci);
		while (1)
		{
			for (; c != '\0' && color != CYAN_TEXT; c = pt_iterate(&pi), color = ci_iterate(&ci)) {}
			for (; c == ' ' || c == '\n'; c = pt_iterate(&pi), color = ci_iterate(&ci)) {}
			
			if (c == '\0')
			{
				break;
			}
			if (color != CYAN_TEXT)
			{
				continue;
			}

			DynamicArray* str = da_create(ARBITRARY_VALUE);
			if (str == NULL)
			{
				continue;
			}

			int i = 0;
			while (color == CYAN_TEXT && c != '\0' && c != ' ' && c != '\n')
			{
				da_insert(str, c, i);
				i++;
				c = pt_iterate(&pi);
				color = ci_iterate(&ci);
			}
			da_insert(str, '\0', i);

			LinkedList* existing_values = hm_get_dangerous(found, str->arr, &hash_function);
			if (existing_values != NULL)
			{
				bool found = false;
				for (int i = 0; i < existing_values->size; i++)
				{
					HashMapElt* elt = ll_get_elt(existing_values, i);
					if (elt != NULL)
					{
						if (!strcmp(elt->key, str->arr))
						{
							found = true;
							break;
						}
					}
				}
				if (found)
				{
					da_free(str);
					continue;
				}
			}

			hm_insert(found, str->arr, NULL, &hash_function);
			ll_insert(lst, str->arr, lst->size);
			free(str);

			if (c == '\0')
			{
				break;
			}
		}
	}

	char** r = malloc(sizeof(char*) * lst->size);
	*arr_len = lst->size;
	int i = 0;
	while (lst->size > 0)
	{
		r[i] = ll_rm(lst, 0);
		i++;
	}
	ll_free(lst);
	hm_free(found, NULL, NULL);
	return r;
}
