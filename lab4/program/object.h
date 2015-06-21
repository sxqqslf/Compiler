#ifndef _OBJECT_H
#define _OBJECT_H

#include "node.h"
#include "intercode.h"

typedef struct _reg {
	char name[2];
	char *variable;
} reg;

typedef struct _var {
	char name[100];
	reg regi;
	int offset;
	struct _var *next;
} var;

reg regi[32];
var *varList;
extern reg *get_reg() ;
extern reg *Ensure(Operand ) ;
extern reg *Allocate(Operand ) ;
extern var *findvar(var *, Operand ) ;
extern var *createvar(Operand , FILE * ) ;
extern void initial(FILE * ) ;
extern void insertvar(var * ) ;
extern void assemblecode(InterCodes *, FILE * ) ;
extern void print_label(InterCodes *, FILE * ) ;
extern void print_assign(InterCodes *, FILE * ) ;
extern void print_binop(int, InterCodes *, FILE* ) ;
extern void print_GOTOS(InterCodes *, FILE * ) ;
extern void print_CALL(InterCodes *, FILE * ) ;
extern void print_RETURN(InterCodes *, FILE * ) ;
extern void print_COND(InterCodes *, FILE * ) ;
extern void print_ARRAY(InterCodes *, FILE * ) ;
extern InterCodes *print_FUNCTION(InterCodes *, FILE * ) ;

#endif
