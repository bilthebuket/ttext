gcc -g -O0 -Wall -Wextra -Wpedantic -Werror -g -o ttext src/ttext.c src/global.c src/io_tools.c src/normal_mode.c src/insert_mode.c src/terminal_mode.c src/tab.c src/linked_list.c src/line.c src/tree.c src/piece_table/piece_table.c src/piece_table/color_indices.c src/piece_table/undo.c src/finder.c src/hash_map.c src/signature.c src/undo.c -Iinclude -lm -lncurses -lpthread

