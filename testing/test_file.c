#include <stdio.h>

void foo(int* i, double d)
{
	*i += (int) d;
}

int main(void)
{
	int x = 5;
	void (*ptr)(int*, double);
	ptr = &foo;
	x = 2 * 5;
	x += 2 + 2;
	x *= 5;
}
