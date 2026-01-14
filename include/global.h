#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdbool.h>
#include "LL.h"
#include "tab.h"
#include "semaphore.h"

#define LINE_SIZE 2048
#define FNAME_SIZE 256
#define TAB_SIZE 4

#define BACKSPACE_KEYCODE1 8
#define BACKSPACE_KEYCODE2 0x7f
#define ESCAPE_KEYCODE 27
#define ENTER_KEYCODE1 10

#define CHANGES_SAVED (1ULL << 63)

#define WHITE_TEXT 1
#define GREEN_TEXT 2
#define BLUE_TEXT 3
#define RED_TEXT 4
#define MAGENTA_TEXT 5
#define YELLOW_TEXT 6
#define CYAN_TEXT 7

#define NUM_DATA_TYPES 12
#define NUM_CONTROL_WORDS 10
#define NUM_LITERALS 3

#define CSI_ESC 1
#define OSC_ESC (1 << 1)
#define SC_ESC (1 << 2)

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
extern char* listener_buf;

extern bool terminate;

extern const char* const data_types[NUM_DATA_TYPES];
extern const char* const control_words[NUM_CONTROL_WORDS];
extern const char* const literals[NUM_LITERALS];

#endif
