#include <stdio.h>
#include <string.h>
#include "intercode.h"
#include "node.h"

int main(int argc, char **argv) {
	if (argc < 3) {
		printf("Usage: ./parser test.c out.s\n");
		exit(0);
	}

	init();
	FILE *f = fopen(argv[1], "r");
	if (!f) {
		perror(argv[1]); return 1;
	}

	root = NULL; 
	yyrestart(f); 
	yyparse();
	close(f);

	f = fopen(argv[2], "w");
	travel(root);
	INIT();
	st = translate(root);
	optimal(st);
	removeLabel(st);
	assemblecode(st, f);
	close(f);

	return 0;
}

yyerror(char *msg) {
	printf("syntax error!\n");
}
