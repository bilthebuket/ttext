#include "global.h"

void (*mode)(int);
LL* tabs;
Tab* active_tab;
int active_tab_index;
Tab* terminal;

int height;
int width;

int slave_pid;
int master_fd;

sem_t sem;
