#include <stdio.h>
#include <stdlib.h>

int main() {
	char *s = malloc(100);
	int a = 100;
	s = itoa(a);
	printf("%s", s);
}
