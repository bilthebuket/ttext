#ifndef GLOBAL_H
#define GLOBAL_H

#include "LL.h"
#include "tab.h"
#include "semaphore.h"

#define LINE_SIZE 2048

#define BACKSPACE_KEYCODE1 8
#define BACKSPACE_KEYCODE2 0x7f
#define ESCAPE_KEYCODE 27
#define ENTER_KEYCODE1 10

extern void (*mode)(int);
extern LL* tabs;
extern Tab* active_tab;
extern int active_tab_index;
extern Tab* terminal;

extern int height;
extern int width;

extern int slave_pid;
extern int master_fd;

extern sem_t sem;

#endif
