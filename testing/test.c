#include <stdio.h>
#include <stdlib.h>
#include "line.h"
#include "global.h"

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		char* buf = malloc(sizeof(char) * LINE_SIZE);
		int i;
		for (i = 0; argv[1][i] != '\0'; i++)
		{
			buf[i] = argv[1][i];
		}
		buf[i] = '\0';
		GapBuffer* gb = gb_create(buf, LINE_SIZE);
		gb_goright(gb);
		gb_put(gb, 'e');
		for (int i = 0; gb_get(gb, i) != '\0'; i++)
		{
			printf("%d ", gb_get(gb, i));
		}
		printf("\n");
	}
}
