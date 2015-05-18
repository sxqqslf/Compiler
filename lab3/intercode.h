#ifndef _INTERCODE_H
#define _INTERCODE_H

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define ASSIGN 0
#define ADD 1
#define SUB 2
#define MUL 3
#define DIV 4
#define MINUS 5
#define ARRAY 6
#define FUNCTION 7

#define VARIABLE 0
#define CONSTINT 1
#define CONSTFLOAT 2
#define TEMPVAR 3

char opt[20] = {' ', '+', '-', '*', '/', '-'};

typedef struct Operand_* Operand;
struct Operand_ {
	int kind;
	union {
		int var_no;
		union { int Int; float Float; } value;
	} u;
	Operand next;
};

typedef struct InterCode {
	int kind;
	union {
		struct { Operand right, left; } assign;
		struct { Operand result, op1, op2; } binop;
		struct { Operand result, op; } minus;
		Operand op;					//函数调用
	} u;
	char name[50];
} InterCode;

typedef struct InterCodes {
	struct InterCode code;
	struct InterCodes *prev, *next;
} InterCodes;

struct InterCodes *st;
int varcount = 1, tmpvarcount = 1;

extern void translate(struct node *) ;
extern InterCodes *translate_Exp(struct node *, Operand ) ;
extern InterCodes *translate_Cond(struct node *, Operand, Operand ) ;
extern InterCodes *translate_Stmt(struct node * ) ;
extern InterCodes *translate_Compst(struct node * ) ;
extern InterCodes *translate_Args(struct node *, FieldList ) ;
extern InterCodes *bindCode(InterCodes, InterCodes ) ;
extern InterCodes *newInterCode(int, Operand, Operand, Operand ) ;
extern Operand newOperand(int, char * ) ;
extern struct void printCode(struct InterCodes * ) ;



struct InterCode createcode(int kind, Operand op1, Operand op2, Operand res) {
	struct InterCode ret;
	if (kind == ASSIGN) {
		ret.kind = kind;
		ret.u.assign.right = op1;
		ret.u.assign.left = op2;
	} else if (kind == ADD || kind == SUB || kind == MUL || kind == DIV) {
		ret.kind = ADD;
		ret.u.binop.op1 = op1;
		ret.u.binop.op2 = op2;
		ret.u.binop.result = res;
	} else if (kind == MINUS) {
		ret.kind = MINUS;
		ret.u.minus.result = res;
		ret.u.minus.op = op1;
	}
	
	return ret;
}


char *change(Operand op) {
	char *ret = malloc(100);
	memset(ret, 0, 100);
	if (op->kind == VARIABLE) {
		char str[20];
		sprintf(str, "%d", op->u.var_no);
		ret[0] = 'v'; strcat(ret+1, str);
	} else if (op->kind == CONSTANT) { 
		char str[20];
		sprintf(str, "%d", op->u.value);
		ret[0] = '#'; strcat(ret+1, str);
	}

	return ret;
}

void printcode(struct InterCodes *st) {
	while (st != NULL) {
		if (st->code.kind == ASSIGN) {
			Operand op1 = st->code.u.assign.right;
			Operand op2 = st->code.u.assign.left;
			char *s1 = change(op1), *s2 = change(op2); 

			printf("%s := %s\n", s1, s2); 
		} else if (st->code.kind >= ADD && st->code.kind <= DIV) {
			Operand op1 = st->code.u.binop.op1;
			Operand op2 = st->code.u.binop.op2;
			Operand res = st->code.u.binop.result;
			char *s1 = change(op1), *s2 = change(op2), *s3 = change(res);

			printf("%s := %s %c %s\n", s3, s1, opt[st->code.kind], s2);
		} else if (st->code.kind == MINUS) {
			Operand op = st->code.u.minus.op;
			Operand res = st->code.u.minus.result;
			char *s1 = change(op), *s2 = change(res);

			printf("%s := -%s\n", s2, s1);
		}
		st = st->next;
	}
}
#endif
