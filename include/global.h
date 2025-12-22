#ifndef GLOBAL_H
#define GLOBAL_H

#include "LL.h"
#include "tab.h"

extern void (*mode)(int);
extern LL* tabs;
extern Tab* active_tab;

extern int height;
extern int width;

#endif
