#ifndef _HEADERNAME_H
#define _HEADERNAME_H

#include <stdio.h>
#include <math.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define HASHSIZE 0x3fff

struct node {
	int	isTerminal;				//是否为终结符号
	int line;					//第一次出现的行号
	char *type;					//类型（ID,INT,Program....）
	char *value;					//属性值
	struct node *child;
	struct node *next;
};

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

typedef struct Type_ {
	enum { BASIC, ARRAY, STRUCTURE } kind;
	union {
		union { int intType; float floatType; } basic;
		struct { Type elem; int size; } array; 
		FieldList structure;
	} u;
} Type_;

typedef struct FieldList_ {
	char *name;					//变量名称
	int lineno;					//行号
	Type type;					//变量类型
	FieldList tail;				//下一个变量
	FieldList head; 			//上一个变量
	FieldList down;				//纵向的下一个变量
} FieldList_;

typedef struct FunctionMessage {
	char *name; 				//函数名称
	int lineno;					//行号
	int visitedTag;				//0:声明,1:定义
	Type_ returnType;			//返回值类型
	FieldList argList;			//参数列表
	FunctionMessage *next;		//下一个函数
} FunctionMessage;

FunctionMessage funcHashtable[HASHSIZE];
FieldList varHashtable[HASHSIZE];
FieldList stack[HASHSIZE];		//其实不是这个size，懒得定义一个新的变量
int top;						//栈指针
extern struct node *newNode(int, int, char*, char* );
extern struct node *createNode(int, ...) ;
extern void printTree(struct node*, int) ;
extern unsigned int hash_pjw(char* ) ;

#endif
