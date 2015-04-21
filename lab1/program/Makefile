all:
	bison -d -v syntax.y
	flex lexical.l
	gcc syntax.tab.c node.c -lfl -ly -o parser -lm

