#ifndef GLOBAL_H
#define GLOBAL_H

#include "LL.h"
#include "tab.h"

#define LINE_SIZE 2048
#define BACKSPACE_KEYCODE1 8
#define BACKSPACE_KEYCODE2 0x7f
#define ESCAPE_KEYCODE 27

extern void (*mode)(int);
extern LL* tabs;
extern Tab* active_tab;

extern int height;
extern int width;

#endif
