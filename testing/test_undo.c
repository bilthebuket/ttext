#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "line.h"
#include "global.h"
#include "tree.h"
#include "piece_table/piece_table.h"
#include <criterion/criterion.h>

EditorState es;

void simulate_insert(const char* str)
{
	(*es.mode)(&es, 'i');
	for (int i = 0; str[i] != '\0'; i++)
	{
		(*es.mode)(&es, str[i]);
	}
	(*es.mode)(&es, ESCAPE_KEYCODE);
}

void simulate_append(const char* str)
{
	(*es.mode)(&es, 'a');
	for (int i = 0; str[i] != '\0'; i++)
	{
		(*es.mode)(&es, str[i]);
	}
	(*es.mode)(&es, ESCAPE_KEYCODE);
}

void goto_coords(int x, int y)
{
	es.active_tab->x = x;
	es.active_tab->y = y;
}

void print_state()
{
	for (int i = 0; pt_get(es.active_tab->pt, i) != '\0'; i++)
	{
		printf("%c", pt_get(es.active_tab->pt, i));
	}
	printf("\n");
}

void expect_state(const char* str)
{
	bool passed = true;

	int i = 0;
	for (; str[i] != '\0'; i++)
	{
		cr_expect_eq(str[i], pt_get(es.active_tab->pt, i));
		if (str[i] != pt_get(es.active_tab->pt, i))
		{
			printf("%d | %d\n", (int) str[i], (int) pt_get(es.active_tab->pt, i));
			passed = false;
		}
	}
	cr_expect_eq(str[i], pt_get(es.active_tab->pt, i));
	if (str[i] != pt_get(es.active_tab->pt, i))
	{
		printf("%d | %d\n", (int) str[i], (int) pt_get(es.active_tab->pt, i));
		passed = false;
	}

	if (!passed)
	{
		printf("expected:\n");
		printf("%s\n", str);
		printf("found:\n");
		print_state();
		fflush(stdout);
		exit(1);
	}
}

void setup_state(void)
{
	es_init(&es, 1, NULL);
}

void teardown_state(void)
{
	es_uninit(&es);
}

Test(editor, test_undo_simple1, .init = setup_state, .fini = teardown_state)
{
	simulate_insert("int main(void)\n{\n\n}");

	goto_coords(0, 1);
	(*es.mode)(&es, '$');

	simulate_append("int x = 5;");

	goto_coords(3, 1);
	(*es.mode)(&es, 'x');
	goto_coords(4, 1);
	(*es.mode)(&es, 'x');

	(*es.mode)(&es, 'u');
	(*es.mode)(&es, 'u');
	(*es.mode)(&es, 'u');

	expect_state("int main(void)\n{\n    \n}");
}

Test(editor, test_undo_simple2, .init = setup_state, .fini = teardown_state)
{
	simulate_insert("int x = 5;");

	goto_coords(0, 0);
	(*es.mode)(&es, 'x');
	goto_coords(0, 2);
	(*es.mode)(&es, 'x');

	goto_coords(0, 0);
	simulate_insert("const ");

	goto_coords(0, 1);
	(*es.mode)(&es, 'x');

	(*es.mode)(&es, 'u');
	(*es.mode)(&es, 'u');
	(*es.mode)(&es, 'u');
	(*es.mode)(&es, 'u');

	expect_state("int x = 5;");
}

Test(editor, test_undo_complex1, .init = setup_state, .fini = teardown_state)
{
	simulate_insert("int main(void)\n{\n\n}");

	goto_coords(0, 2);
	(*es.mode)(&es, '$');

	simulate_append("for()");

	(*es.mode)(&es, 'h');
	simulate_insert("int i = 0; i < 10; i++");

	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'x');

	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'x');

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    for(int i = 0; i < 10; i+)\n}");
	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    for(int i = 0; i < 10; i++)\n}");
	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    for()\n}");
	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    \n}");

	goto_coords(0, 2);
	(*es.mode)(&es, '$');
	simulate_append("if(true)\n{\n\n}");

	goto_coords(0, 4);
	(*es.mode)(&es, '$');
	simulate_append("printf(str)");

	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'x');

	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'x');

	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'x');

	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'x');

	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'x');

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    if(true)\n    {\n        print()\n    }\n}");
	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    if(true)\n    {\n        printf()\n    }\n}");
	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    if(true)\n    {\n        printf(s)\n    }\n}");
	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    if(true)\n    {\n        printf(st)\n    }\n}");
	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    if(true)\n    {\n        printf(str)\n    }\n}");
	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    if(true)\n    {\n        \n    }\n}");
	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    \n}");
	(*es.mode)(&es, 'u');
	expect_state("");
}
