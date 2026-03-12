#include <stddef.h>
#include "global.h"

Tab* (*mode)(Tab*, int) = NULL;
LL* tabs = NULL;
int active_tab_index;

int height;
int width;

sem_t sem;

bool terminate = false;

FILE* error_log = NULL;
