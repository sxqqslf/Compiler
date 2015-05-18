all:
	bison -d -v syntax.y
	flex lexical.l
	gcc semtanic.c syntax.tab.c node.c -lfl -ly -o parser -lm -g

