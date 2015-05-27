#ifndef _INTERCODE_H
#define _INTERCODE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "node.h"

#define ASSIGN 0
#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4
#define ARRAY 5 
#define FUNCTION 6
#define LABEL 7 
#define CONDITION 8
#define RETURN 9
#define ARG 10
#define GOTOL 11
#define CALL 12
#define PARAM 13
#define DEC 14

#define VARIABLE 0
#define CONSTINT 1
#define CONSTFLOAT 2
#define TEMPVAR 3
#define OPLABEL 4
#define OPCONDITION 5
#define TEMPVAR_ 6

typedef struct Operand_* Operand;
typedef struct Operand_ {
	int kind;
	union {
		int var_no;
		union { int Int; float Float; char relop[3]; } value;
	} u;
	Operand next;
} Operand_;

typedef struct InterCode {
	int kind;
	union {
		struct { Operand right, left; } assign;
		struct { Operand result, op1, op2; } binop;
		struct { Operand result, op; } minus;
		struct { Operand ID, addr; } array;
		struct { Operand op1, op2, rel, label; } relop;
		Operand op;					//函数调用或label或函數返回
	} u;
	char name[50];
} InterCode;

typedef struct InterCodes {
	struct InterCode code;
	struct InterCodes *prev, *next;
} InterCodes;

struct InterCodes *st;
extern int varcount;
extern int tmpvarcount;
extern int labelcount;

extern void INIT();
extern InterCodes *translate(struct node * ) ;
extern InterCodes *translate_Exp(struct node *, Operand ) ;
extern InterCodes *translate_Cond(struct node *, Operand, Operand ) ;
extern InterCodes *translate_Stmt(struct node * ) ;
extern InterCodes *translate_Compst(struct node * ) ;
extern InterCodes *translate_Args(struct node *, Operand * ) ;
extern InterCodes *translate_VarDec(struct node *) ;
extern InterCodes *bindCode(InterCodes *, InterCodes * ) ;
extern InterCodes *newInterCode(int, Operand, Operand, Operand ) ;
extern Operand bindOperand(Operand, Operand ) ;
extern Operand newOperand(int, char * ) ;
extern int find(char * , int ) ;
extern void printCode(struct InterCodes * , FILE * ) ;
extern void optimal(InterCodes * ) ;
extern void delCode(InterCodes * ) ;
extern void removeLabel(InterCodes * ) ;
extern void removeDul(InterCodes * ) ;
extern int same(InterCodes *, InterCodes * ) ;

#endif
