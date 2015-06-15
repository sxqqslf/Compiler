all:
	bison -d -v syntax.y
	flex lexical.l
	gcc semtanic.c syntax.tab.c node.c intercode.c main.c -lfl -ly -o parser -lm -g

