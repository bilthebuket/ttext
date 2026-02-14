--> required libraries <--
ncurses

--> compile <--
bash compile.sh

--> run <--
./ttext <filename1> <filename2> ...

--> use <--
normal mode:
hjkl - left down up right
x - delete character
0 - move to first character in line
$ - move to last character in line
% - goto matching grouping symbol
i - enter insert mode
a - move cursor one to the right and enter insert mode
t - enter terminal mode

insert mode:
default - insert character
esc - enter normal mode

terminal mode:
default - insert character
esc - enter normal mode
custom commands:
:tabnew <filename> - create new tab
:tabn - next tab
:tabp - previous tab
:tab <number> - switch to specified tab
:rs <top/bottom/left/right> <add/sub> <value> - change the size of an edge of the current tab
:mv <up/down/left/right> <value> - move current tab
:q - quit
:q! - force quit (dont check if tab has been written to disk)
:w - write tab to disk
