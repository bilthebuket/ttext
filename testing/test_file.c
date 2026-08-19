#include <stdio.h>
// test
#include "some_stuff.h"

// test
#define SOME_CONSTANT 54

// test
void foo(int* i, double d)
{
	// test
	*i += (int) d; // test
}

int main(void)
{
	int x = 5;
	void (*ptr)(int*, double);
	ptr = &foo;
	x = 2 * 5;
	x += 2 + 2;
	x *= 5;
	int* ptr2 = &x;
	(*ptr)(&x, (double) *ptr2);

	x += 'a';
	printf("test\n");
}
