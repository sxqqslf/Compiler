#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

struct test {
	int a;
};

int main() {
	int *a = malloc(sizeof(int));
	free(a);
	assert(a == NULL);
}
