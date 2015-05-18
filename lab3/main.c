#include <stdio.h>
#include <string.h>
#include "intercode.h"
#include "node.h"

int main(int argc, char **argv) {
	init();
	FILE *f = fopen(argv[1], "r");
	if (!f) {
		perror(argv[1]); return 1;
	}

	root = NULL; 
	yyrestart(f); 
	yyparse();

	travel(root);
	translate(root);

	return 0;
}

yyerror(char *msg) {
	printf("syntax error!\n");
}
