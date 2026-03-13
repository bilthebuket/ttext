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
o - make newline below current line and enter insert mode
t - enter terminal mode

insert mode:
default - insert character
esc - enter normal mode

terminal mode:
default - insert character
enter - send command
backspace - delete character
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
any command without a ':' prefix will be run as a bash command (ls, cd, mv, cat, grep, etc)

Styling Guide:

function names - example_function(args)

struct names - ExampleStruct
all structs should use typedef so that you don't need to type struct <struct_name>

for functions involving a particular struct, it should be the struct abbreviation followed by the function purpose
example: gb_create(args) -> creates a GapBuffer
preferred grammar:
-abv_create() - create struct
-abv_free() - free/destroy struct
-abv_get() - get element from struct
-abv_insert() - add element to struct
-abv_rm() - remove element from struct

braces:
function()
{

}

comments - any way of typing comments is acceptable (ex //, /* */), if the purpose of a block of code/function name, argument, or return value
is not easily understandable, it should be commented

constants:
EXAMPLE_CONSTANT
should be #define'd in include/global.h, unless it's only used in one source/header file

variables:
example_variable, ex_var
abbreviations are acceptable as long as it is clear what the variable does

filenames - file_name.extension
