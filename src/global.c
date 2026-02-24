#include <stddef.h>
#include "global.h"

Tab* (*mode)(Tab*, int);
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

const char* const data_types[NUM_DATA_TYPES] = {"int", "long", "float", "double", "char", "const", "extern", "void", "pthread_t", "sem_t", "bool", "static", "unsigned", "struct"};
const char* const control_words[NUM_CONTROL_WORDS] = {"if", "else", "while", "for", "switch", "case", "break", "continue", "default", "return", "typedef"};
const char* const literals[NUM_LITERALS] = {"true", "false", "NULL"};

FILE* error_log = NULL;
