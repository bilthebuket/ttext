#ifndef FUZZY_FIND_H
#define FUZZY_FIND_H

void fuzzy_find_init(void);
void order_by_closest_match(char** to_order, char* target);
char** find_strings_to_autocomplete(EditorState* es, int* arr_len);

#endif
