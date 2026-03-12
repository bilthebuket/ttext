#ifndef GLOBAL_H
#define GLOBAL_H

#include <stdio.h>
#include <stdbool.h>
#include "LL.h"
#include "tab.h"
#include "semaphore.h"

#define LINE_SIZE 2048
#define FNAME_SIZE 256
#define GB_SIZE 256
#define TAB_SIZE 4
#define APPEND_SIZE 8192

#define BACKSPACE_KEYCODE1 8
#define BACKSPACE_KEYCODE2 0x7f
#define ESCAPE_KEYCODE 27
#define ENTER_KEYCODE1 10

#define CHANGES_SAVED (1 << 31)
#define FLAG_BITS (1 << 31)
#define TAB_NUM_BITS ~FLAG_BITS

#define WHITE_TEXT 1
#define GREEN_TEXT 2
#define BLUE_TEXT 3
#define RED_TEXT 4
#define MAGENTA_TEXT 5
#define YELLOW_TEXT 6
#define CYAN_TEXT 7

#define NUM_DATA_TYPES 14
#define NUM_CONTROL_WORDS 11
#define NUM_LITERALS 3

#define CSI_ESC 1
#define OSC_ESC (1 << 1)
#define SC_ESC (1 << 2)

#define NUM_CHARS 128

extern Tab* (*mode)(Tab*, int);

// maintained in ascending order of z_index (last element in list is on top of screen)
extern LL* tabs;
extern int active_tab_index;

// height and width of the screen
extern int height;
extern int width;

extern sem_t sem;

extern bool terminate;

extern FILE* error_log;

#endif
