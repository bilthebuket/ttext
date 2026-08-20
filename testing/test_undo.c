#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <time.h>
#include <math.h>
#include "line.h"
#include "global.h"
#include "tree.h"
#include "piece_table/piece_table.h"
#include "piece_table/color_indices.h"
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
void print_chars(int num)
{
	for (int i = 0; i < num; i++)
	{
		if (pt_get(es.active_tab->pt, i) == '\0')
		{
			printf("^");
		}
		else
		{
			printf("%c", pt_get(es.active_tab->pt, i));
		}
	}
	printf("\n");
}

void signature_expect_state(const char** signatures)
{
	bool passed = true;
	int i = 0;
	for (; strcmp(signatures[i], "\0"); i += 2)
	{
		LinkedList* lst = hm_get_dangerous(es.signatures, signatures[i], &hash_function);
		if (lst == NULL)
		{
			passed = false;
			printf("%s | %s: no signatures found for function name\n", signatures[i], signatures[i + 1]);
			continue;
		}

		bool found = false;
		for (int j = 0; j < lst->size; j++)
		{
			HashMapElt* hme = ll_get_elt(lst, j);
			if (hme != NULL)
			{
				if (!strcmp((char*) hme->key, signatures[i]) && !strcmp(((Signature*) hme->value)->signature, signatures[i + 1]))
				{
					found = true;
					break;
				}
			}
		}

		if (!found)
		{
			passed = false;
			printf("%s | %s: signatures found matching function name, but not given signature\n", signatures[i], signatures[i + 1]);
		}
	}

	i++;
	for (; strcmp(signatures[i], "\0"); i++)
	{
		LinkedList* lst = hm_get_dangerous(es.signatures, signatures[i], &hash_function);
		if (lst != NULL && lst->size > 0)
		{
			passed = false;
			printf("the following signatures found for function name %s which was expected to have no signatures:\n", signatures[i]);
			for (int j = 0; j < lst->size; j++)
			{
				HashMapElt* hme = ll_get_elt(lst, j);
				printf("%s\n", ((Signature*) hme->value)->signature);
			}
		}
	}

	if (!passed)
	{
		fflush(stdout);
		exit(1);
	}
}

void ci_expect_state(const int* colors)
{
	bool passed = true;

	int i = 0;
	char c = pt_get(es.active_tab->pt, 0);
	int color = colors[1];
	int num_chars_remaining = colors[0];
	int colors_covered = 0;
	for (; c != '\0'; i++, c = pt_get(es.active_tab->pt, i))
	{
		if (c != ' ' && c != '\n')
		{
			if (pt_get_color(es.active_tab->pt, i) != color)
			{
				passed = false;
				printf("index %d char %c: expected %d | found %d\n", i, c, color, pt_get_color(es.active_tab->pt, i));
			}
			num_chars_remaining--;
			if (num_chars_remaining == 0)
			{
				colors_covered++;
				color = colors[colors_covered * 2 + 1];
				num_chars_remaining = colors[colors_covered * 2];
			}
		}
	}

	if (!passed)
	{
		fflush(stdout);
		exit(1);
	}
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
			printf("index %d, expected %d | found %d\n", i, (int) str[i], (int) pt_get(es.active_tab->pt, i));
			passed = false;
		}
	}
	cr_expect_eq(str[i], pt_get(es.active_tab->pt, i));
	if (str[i] != pt_get(es.active_tab->pt, i))
	{
		printf("index %d, expected %d | found %d\n", i, (int) str[i], (int) pt_get(es.active_tab->pt, i));
		passed = false;
	}

	if (!passed)
	{
		printf("expected:\n");
		printf("%s\n", str);
		printf("found:\n");
		print_chars(i);
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
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, CYAN_TEXT, 3, YELLOW_TEXT});
}

Test(editor, test_undo_simple2, .init = setup_state, .fini = teardown_state)
{
	simulate_insert("int x = 5;");
	expect_state("int x = 5;");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 1, CYAN_TEXT, 3, RED_TEXT});

	goto_coords(0, 0);
	(*es.mode)(&es, 'x');
	expect_state("nt x = 5;");
	ci_expect_state((const int[]) {2, BLUE_TEXT, 1, CYAN_TEXT, 3, RED_TEXT});

	goto_coords(2, 0);
	(*es.mode)(&es, 'x');
	expect_state("ntx = 5;");
	ci_expect_state((const int[]) {3, CYAN_TEXT, 3, RED_TEXT});

	goto_coords(0, 0);
	simulate_insert("const ");
	expect_state("const ntx = 5;");
	ci_expect_state((const int[]) {5, BLUE_TEXT, 3, CYAN_TEXT, 3, RED_TEXT});

	goto_coords(1, 0);
	(*es.mode)(&es, 'x');
	expect_state("cnst ntx = 5;");
	ci_expect_state((const int[]) {4, BLUE_TEXT, 3, CYAN_TEXT, 3, RED_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("const ntx = 5;");
	ci_expect_state((const int[]) {5, BLUE_TEXT, 3, CYAN_TEXT, 3, RED_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("ntx = 5;");
	ci_expect_state((const int[]) {3, CYAN_TEXT, 3, RED_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("nt x = 5;");
	ci_expect_state((const int[]) {2, BLUE_TEXT, 1, CYAN_TEXT, 3, RED_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("int x = 5;");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 1, CYAN_TEXT, 3, RED_TEXT});
}

Test(editor, test_undo_complex1, .init = setup_state, .fini = teardown_state)
{
	simulate_insert("int main(void)\n{\n\n}");

	goto_coords(0, 2);
	(*es.mode)(&es, '$');

	simulate_append("for()");

	(*es.mode)(&es, 'h');
	simulate_append("int i = 0; i < 10; i++");

	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'x');

	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'h');
	(*es.mode)(&es, 'x');

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    for(int i = 0; i < 10; i+)\n}");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT, 2, YELLOW_TEXT, 3, MAGENTA_TEXT, 1, YELLOW_TEXT, 3, BLUE_TEXT,
				       1, CYAN_TEXT, 3, RED_TEXT, 1, CYAN_TEXT, 4, RED_TEXT, 1, CYAN_TEXT, 1, RED_TEXT, 2, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    for(int i = 0; i < 10; i++)\n}");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT, 2, YELLOW_TEXT, 3, MAGENTA_TEXT, 1, YELLOW_TEXT, 3, BLUE_TEXT,
					1, CYAN_TEXT, 3, RED_TEXT, 1, CYAN_TEXT, 4, RED_TEXT, 1, CYAN_TEXT, 2, RED_TEXT, 2, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    for()\n}");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT, 2, YELLOW_TEXT, 3, MAGENTA_TEXT, 3, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    \n}");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT, 3, YELLOW_TEXT});

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
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT, 2, YELLOW_TEXT, 2, MAGENTA_TEXT,
					1, YELLOW_TEXT, 4, CYAN_TEXT, 2, YELLOW_TEXT, 9, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    if(true)\n    {\n        printf()\n    }\n}");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT, 2, YELLOW_TEXT, 2, MAGENTA_TEXT,
					1, YELLOW_TEXT, 4, CYAN_TEXT, 2, YELLOW_TEXT, 10, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    if(true)\n    {\n        printf(s)\n    }\n}");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT, 2, YELLOW_TEXT, 2, MAGENTA_TEXT,
					1, YELLOW_TEXT, 4, CYAN_TEXT, 9, YELLOW_TEXT, 1, CYAN_TEXT, 3, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    if(true)\n    {\n        printf(st)\n    }\n}");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT, 2, YELLOW_TEXT, 2, MAGENTA_TEXT,
					1, YELLOW_TEXT, 4, CYAN_TEXT, 9, YELLOW_TEXT, 2, CYAN_TEXT, 3, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    if(true)\n    {\n        printf(str)\n    }\n}");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT, 2, YELLOW_TEXT, 2, MAGENTA_TEXT,
					1, YELLOW_TEXT, 4, CYAN_TEXT, 9, YELLOW_TEXT, 3, CYAN_TEXT, 3, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    if(true)\n    {\n        \n    }\n}");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT, 2, YELLOW_TEXT, 2, MAGENTA_TEXT,
					1, YELLOW_TEXT, 4, CYAN_TEXT, 4, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("int main(void)\n{\n    \n}");
	ci_expect_state((const int[]) {3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT, 3, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("");
}

Test(editor, test_undo_complex2, .init = setup_state, .fini = teardown_state)
{
	simulate_insert("#include <stdio.h>");
	(*es.mode)(&es, 'o');
	(*es.mode)(&es, ESCAPE_KEYCODE);
	simulate_append("#include <stdlib.h>");

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT});

	goto_coords(0, 0);
	(*es.mode)(&es, '$');

	simulate_append("\n\nint main(void)\n{\nprintf();\n}");
	goto_coords(10, 4);
	simulate_append("test");

	goto_coords(9, 2);
	for (int i = 0; i < 4; i++) {(*es.mode)(&es, 'x');}
	simulate_insert("int argc, char* argv[]");
	
	goto_coords(15, 4);
	(*es.mode)(&es, 'i');
	for (int i = 0; i < 4; i++) {(*es.mode)(&es, BACKSPACE_KEYCODE2);}
	(*es.mode)(&es, ESCAPE_KEYCODE);
	simulate_append("argc");

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(int argc, char* argv[])\n{\n    printf();\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 3, BLUE_TEXT, 4, CYAN_TEXT, 1, RED_TEXT,
					5, BLUE_TEXT, 4, CYAN_TEXT, 12, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});
	goto_coords(10, 4);
	simulate_append("argv[i]");

	goto_coords(0, 3);
	(*es.mode)(&es, 'o');
	(*es.mode)(&es, ESCAPE_KEYCODE);
	simulate_append("for (int i = 1; i < argc; i++)\n{");
	
	goto_coords(4, 6);
	simulate_insert("\t");
	(*es.mode)(&es, 'o');
	(*es.mode)(&es, ESCAPE_KEYCODE);
	simulate_append("}");
	expect_state("#include <stdio.h>\n\nint main(int argc, char* argv[])\n{\n    for (int i = 1; i < argc; i++)\n    {\n        printf(argv[i]);\n    }\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 3, BLUE_TEXT, 4, CYAN_TEXT, 1, RED_TEXT, 5, BLUE_TEXT, 4, CYAN_TEXT,
					4, YELLOW_TEXT, 3, MAGENTA_TEXT, 1, YELLOW_TEXT, 3, BLUE_TEXT, 1, CYAN_TEXT, 3, RED_TEXT, 1, CYAN_TEXT, 1, RED_TEXT,
					4, CYAN_TEXT, 1, RED_TEXT, 1, CYAN_TEXT, 2, RED_TEXT, 9, YELLOW_TEXT, 4, CYAN_TEXT, 1, YELLOW_TEXT, 1, CYAN_TEXT,
					2, YELLOW_TEXT, 1, RED_TEXT, 2, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(int argc, char* argv[])\n{\n    for (int i = 1; i < argc; i++)\n    {\n        printf(argv[i]);\n        \n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 3, BLUE_TEXT, 4, CYAN_TEXT, 1, RED_TEXT, 5, BLUE_TEXT, 4, CYAN_TEXT,
					4, YELLOW_TEXT, 3, MAGENTA_TEXT, 1, YELLOW_TEXT, 3, BLUE_TEXT, 1, CYAN_TEXT, 3, RED_TEXT, 1, CYAN_TEXT, 1, RED_TEXT,
					4, CYAN_TEXT, 1, RED_TEXT, 1, CYAN_TEXT, 2, RED_TEXT, 9, YELLOW_TEXT, 4, CYAN_TEXT, 1, YELLOW_TEXT, 1, CYAN_TEXT,
					2, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(int argc, char* argv[])\n{\n    for (int i = 1; i < argc; i++)\n    {\n        printf(argv[i]);\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 3, BLUE_TEXT, 4, CYAN_TEXT, 1, RED_TEXT, 5, BLUE_TEXT, 4, CYAN_TEXT,
					4, YELLOW_TEXT, 3, MAGENTA_TEXT, 1, YELLOW_TEXT, 3, BLUE_TEXT, 1, CYAN_TEXT, 3, RED_TEXT, 1, CYAN_TEXT, 1, RED_TEXT,
					4, CYAN_TEXT, 1, RED_TEXT, 1, CYAN_TEXT, 2, RED_TEXT, 9, YELLOW_TEXT, 4, CYAN_TEXT, 1, YELLOW_TEXT, 1, CYAN_TEXT,
					2, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(int argc, char* argv[])\n{\n    for (int i = 1; i < argc; i++)\n    {\n    printf(argv[i]);\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 3, BLUE_TEXT, 4, CYAN_TEXT, 1, RED_TEXT, 5, BLUE_TEXT, 4, CYAN_TEXT,
					4, YELLOW_TEXT, 3, MAGENTA_TEXT, 1, YELLOW_TEXT, 3, BLUE_TEXT, 1, CYAN_TEXT, 3, RED_TEXT, 1, CYAN_TEXT, 1, RED_TEXT,
					4, CYAN_TEXT, 1, RED_TEXT, 1, CYAN_TEXT, 2, RED_TEXT, 9, YELLOW_TEXT, 4, CYAN_TEXT, 1, YELLOW_TEXT, 1, CYAN_TEXT,
					2, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(int argc, char* argv[])\n{\n    \n    printf(argv[i]);\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 3, BLUE_TEXT, 4, CYAN_TEXT, 1, RED_TEXT, 5, BLUE_TEXT, 4, CYAN_TEXT,
					11, YELLOW_TEXT, 4, CYAN_TEXT, 1, YELLOW_TEXT, 1, CYAN_TEXT,
					2, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(int argc, char* argv[])\n{\n    printf(argv[i]);\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 3, BLUE_TEXT, 4, CYAN_TEXT, 1, RED_TEXT, 5, BLUE_TEXT, 4, CYAN_TEXT,
					11, YELLOW_TEXT, 4, CYAN_TEXT, 1, YELLOW_TEXT, 1, CYAN_TEXT,
					2, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(int argc, char* argv[])\n{\n    printf();\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 3, BLUE_TEXT, 4, CYAN_TEXT, 1, RED_TEXT, 5, BLUE_TEXT, 4, CYAN_TEXT,
					12, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(int argc, char* argv[])\n{\n    printf(test);\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 3, BLUE_TEXT, 4, CYAN_TEXT, 1, RED_TEXT, 5, BLUE_TEXT, 4, CYAN_TEXT,
					11, YELLOW_TEXT, 4, CYAN_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main()\n{\n    printf(test);\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 14, YELLOW_TEXT,
					4, CYAN_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(d)\n{\n    printf(test);\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 1, CYAN_TEXT, 9, YELLOW_TEXT,
					4, CYAN_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(id)\n{\n    printf(test);\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 2, CYAN_TEXT, 9, YELLOW_TEXT,
					4, CYAN_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(oid)\n{\n    printf(test);\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 3, CYAN_TEXT, 9, YELLOW_TEXT,
					4, CYAN_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(void)\n{\n    printf(test);\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 4, CYAN_TEXT, 9, YELLOW_TEXT,
					4, CYAN_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>\n\nint main(void)\n{\n    printf();\n}");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 4, CYAN_TEXT, 10, YELLOW_TEXT,
					1, RED_TEXT, 1, YELLOW_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("#include <stdio.h>");
	ci_expect_state((const int[]) {8, MAGENTA_TEXT, 9, RED_TEXT});

	(*es.mode)(&es, 'u');
	expect_state("");
}

Test(editor, test_undo_signatures1, .init = setup_state, .fini = teardown_state)
{
	es.active_tab->tab_num_flags |= PARSE_FOR_SIGNATURES;
	simulate_insert("void foo1(void)\n{\n\n}\n\nint main(void)\n{\n\n}\n\nvoid foo2(void)\n{\n\n}");
	signature_expect_state((const char*[]) {"foo1", "void foo1(void)", "main", "int main(void)", "foo2", "void foo2(void)", "\0", "\0"});

	goto_coords(6, 10);
	(*es.mode)(&es, 'x');
	signature_expect_state((const char*[]) {"foo1", "void foo1(void)", "main", "int main(void)", "fo2", "void fo2(void)", "\0", "foo2", "\0"});

	goto_coords(6, 0);
	(*es.mode)(&es, 'x');
	signature_expect_state((const char*[]) {"fo1", "void fo1(void)", "main", "int main(void)", "fo2", "void fo2(void)", "\0", "foo1", "foo2", "\0"});

	(*es.mode)(&es, 'u');
	signature_expect_state((const char*[]) {"foo1", "void foo1(void)", "main", "int main(void)", "fo2", "void fo2(void)", "\0", "fo1", "foo2", "\0"});

	(*es.mode)(&es, 'u');
	signature_expect_state((const char*[]) {"foo1", "void foo1(void)", "main", "int main(void)", "foo2", "void foo2(void)", "\0", "fo1", "fo2", "\0"});

	(*es.mode)(&es, 'u');
	signature_expect_state((const char*[]) {"\0", "foo1", "foo2", "main", "fo1", "fo2", "\0"});
}

Test(editor, test_from_file, .fini = teardown_state)
{
	const char* argv[] = {"./ttext_test", "testing/test_file.c"};
	es_init(&es, 2, (char**) argv);

	ci_expect_state((const int[]) {
		8, MAGENTA_TEXT, 9, RED_TEXT, 6, GREEN_TEXT, 8, MAGENTA_TEXT, 14, RED_TEXT, 6, GREEN_TEXT, 7, MAGENTA_TEXT,
		15, RED_TEXT, 6, GREEN_TEXT, 4, BLUE_TEXT, 4, YELLOW_TEXT, 4, BLUE_TEXT, 1, CYAN_TEXT, 1, RED_TEXT, 6, BLUE_TEXT,
		1, CYAN_TEXT, 2, YELLOW_TEXT, 6, GREEN_TEXT, 1, RED_TEXT, 1, CYAN_TEXT, 2, RED_TEXT, 1, YELLOW_TEXT, 3, BLUE_TEXT,
		1, YELLOW_TEXT, 1, CYAN_TEXT, 1, RED_TEXT, 6, GREEN_TEXT, 1, YELLOW_TEXT, 3, BLUE_TEXT, 5, YELLOW_TEXT, 4, BLUE_TEXT,
		2, YELLOW_TEXT, 3, BLUE_TEXT, 1, CYAN_TEXT, 3, RED_TEXT, 4, BLUE_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT, 3, CYAN_TEXT,
		2, YELLOW_TEXT, 4, BLUE_TEXT, 1, RED_TEXT, 6, BLUE_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT, 3, CYAN_TEXT, 2, RED_TEXT, 3, CYAN_TEXT,
		1, RED_TEXT, 1, CYAN_TEXT, 5, RED_TEXT, 1, CYAN_TEXT, 6, RED_TEXT, 1, CYAN_TEXT, 4, RED_TEXT, 4, BLUE_TEXT, 4, CYAN_TEXT, 
		2, RED_TEXT, 1, CYAN_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT, 3, CYAN_TEXT, 2, YELLOW_TEXT, 1, RED_TEXT, 1, CYAN_TEXT,
		1, RED_TEXT, 1, YELLOW_TEXT, 6, BLUE_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT, 4, CYAN_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT,
		1, CYAN_TEXT, 6, RED_TEXT, 7, YELLOW_TEXT, 8, RED_TEXT, 1, YELLOW_TEXT, 1, RED_TEXT, 1, YELLOW_TEXT
	});
}
