#ifndef _HEADERNAME_H
#define _HEADERNAME_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

struct node {
	int	isTerminal;				//是否为终结符号
	int line;					//第一次出现的行号
	char *type;					//类型（ID,INT,Program....）
	char *value;					//属性值
	struct node *child;
	struct node *next;
};

extern struct node *newNode(int, int, char*, char* );
extern struct node *createNode(int, ...) ;
extern void printTree(struct node*, int) ;
#endif
