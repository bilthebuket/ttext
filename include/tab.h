#ifndef TAB_H
#define TAB_H

#include "LL.h"

typedef struct Tab
{
	LL* lines;
	char* fname;
	int x;
	int y;
} Tab;

Tab* make_tab(char* fname);

#endif
