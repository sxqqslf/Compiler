#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

int main() {
	int *a = malloc(sizeof(int));
	free(a);
	assert(a == NULL);
}
