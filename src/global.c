#include <stddef.h>
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
char* listener_buf = NULL;

bool terminate = false;

const char[NUM_DATA_TYPES][] const data_types = {"int", "long", "float", "double", "char", "const", "extern", "void", "pthread_t", "sem_t", "bool"};
const char[NUM_CONTROL_WORDS][] const control_words = {"if", "else", "while", "for", "switch", "case", "break", "continue"};
const char[NUM_LITERALS][] const literals = {"true", "false", "NULL"};

