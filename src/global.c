#include "global.h"

void (*mode)(int);
LL* tabs;
Tab* active_tab;
Tab* terminal;

int height;
int width;

int slave_pid;
int master_fd;
