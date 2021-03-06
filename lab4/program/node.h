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
	char value[100];					//属性值
	struct node *child;
	struct node *next;
};

typedef struct Type_* Type;
typedef struct FieldList_* FieldList;

typedef struct Type_ {
	enum { Int, Float, Array, Structure } kind;
	union {
		union { int intValue; float floatValue; } basic;
		struct { Type elem; int size; } array; 
		FieldList structure;
	} u;
} Type_;

typedef struct FieldList_ {
	char *name;					//变量名称
	int lineno;					//行号
	int level;
	Type type;					//变量类型
	FieldList tail;				//下一个变量
	FieldList head; 			//上一个变量
	FieldList down;				//纵向的下一个变量
} FieldList_;

typedef struct FunctionMessage {
	char *name; 				//函数名称
	int lineno;					//行号
	int visitedTag;				//0:声明,1:定义
	Type returnType;			//返回值类型
	FieldList argList;			//参数列表
	struct FunctionMessage *next;		//下一个函数
} FunctionMessage;

FunctionMessage *funcHashtable[HASHSIZE];
FieldList varHashtable[HASHSIZE];
FieldList structHashtable[HASHSIZE];
FieldList stack[HASHSIZE];		//其实不是这个size，懒得定义一个新的变量
int top;						//栈指针
int level;
struct node *root;

extern struct node *newNode(int, int, char*, char* );
extern struct node *createNode(int, ...) ;
extern void printTree(struct node*, int) ;
extern unsigned int hash_pjw(char* ) ;

extern void init();
extern int insertVar(FieldList , int );
extern void checkfuncdef();
extern void travel(struct node * );
extern void ExtDefList(struct node * );
extern void ExtDef(struct node * );
extern void ExtDecList(struct node *, Type );
extern Type Specifier(struct node * );
extern Type StructSpecifier(struct node * );
extern FieldList DefList(struct node *, int , FieldList );
extern FieldList Def(struct node *, int );
extern FieldList DecList(FieldList, struct node *, Type, int );
extern FieldList Dec(struct node *, Type, int );
extern void Compst(struct node *, FieldList var, Type, int );
extern void StmtList(struct node *, Type );
extern void Stmt(struct node *, Type );
extern FunctionMessage *FunDec(struct node *, Type , int , FieldList *);
extern FieldList VarList(struct node * );
extern FieldList ParamDec(struct node * );
extern FieldList VarDec(struct node * , Type, int , int );
extern Type Exp(struct node * );
extern FieldList Args(struct node * );
extern int compare(FieldList, FieldList );
extern int compareType(Type, Type );
extern int insertStruct(FieldList );
extern void insertFunc(FunctionMessage * );
extern void copyType(Type, Type );
extern void copyField(FieldList, FieldList );
extern void del(FieldList ); 
extern void dell(Type );
#endif
