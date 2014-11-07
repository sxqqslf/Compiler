%{
	#include <stdio.h>
	#include "node.h"
	node* root;
%}

%union {
	int 	type_int;
	float 	type_float;
	double	type_double;
	node*	type_node;
}

%token <type_node> INT FLOAT ID TYPE
%token <type_node> SEMI COMMA DOT
%token <type_node> ASSIGNOP RELOP GT LT GE LE EQ NE
%token <type_node> PLUS MINUS STAR DIV
%token <type_node> AND OR NOT
%token <type_node> LP RP LB RB LC RC
%token <type_node> STRUCT RETURN IF ELSE WHILE

%type <type_node> Program ExtDefList ExtDef ExtDecList
%type <type_node> Specifier StructSpecifier OptTag Tag
%type <type_node> VarDec FunDec VarList ParamDec
%type <type_node> CompSt StmtList Stmt
%type <type_node> DefList Def DecList Dec Exp Args

%right ASSIGNOP
%left  OR
%left  AND
%left  RELOP LT LE GT GE EQ NE
%left  PLUS MINUS
%left  STAR DIV
%right NOT
%left  LP RP LB RB DOT

%nonassoc LOWER_THAN_ELSE
%nonassoc ELSE

%%

Program		:	ExtDefList						{ $$ = createNode(1, "Program", $1); root = $$; }
			;
ExtDefList	:	ExtDef ExtDefList				{ $$ = createNode(2,"ExtDefList", $1, $2); }
			|									{ $$ = NULL; }
			;
ExtDef		:	Specifier ExtDecList SEMI		{ $$ = createNode(3, "ExtDef", $1, $2, $3); }
			|	Specifier SEMI					{ $$ = createNode(2, "ExtDef", $1, $2); }
			|	Specifier FunDec CompSt			{ $$ = createNode(3, "ExtDef", $1, $2, $3); }
			;
ExtDecList	:	VarDec							{ $$ = createNode(1, "ExtDecList", $1); }
			|	VarDec COMMA ExtDecList			{ $$ = createNode(3, "ExtDecList", $1, $2, $3); }
			;
Specifier	:	TYPE 							{ $$ = createNode(1, "Specifier", $1); }
			|	StructSpecifier					{ $$ = createNode(1, "Specifier", $1); }
			;
StructSpecifier	:	STRUCT OptTag LC DefList RC	{ $$ = createNode(5, "StructSpecifier", $1, $2, $3, $4, $5); }
				|	STRUCT Tag					{ $$ = createNode(2, "StructSpecifier", $1, $2); }
				;
OptTag		:	ID								{ $$ = createNode(1, "OptTag", $1); }
			|									{ $$ = NULL; }
			;
Tag			:	ID								{ $$ = createNode(1, "Tag", $1); }
			;					
VarDec		:	ID								{ $$ = createNode(1, "VarDec", $1); }
			|	VarDec LB INT RB				{ $$ = createNode(4, "VarDec", $1, $2, $3, $4); }
			;
FunDec		:	ID LP VarList RP				{ $$ = createNode(4, "FunDec", $1, $2, $3, $4); }
			|	ID LP RP						{ $$ = createNode(3, "FunDec", $1, $2, $3); }
			;
VarList		:	ParamDec COMMA VarList			{ $$ = createNode(3, "VarList", $1, $2, $3); }
			|	ParamDec						{ $$ = createNode(1, "VarList", $1); }
			;
ParamDec	:	Specifier VarDec				{ $$ = createNode(2, "ParamDec", $1, $2); }
			;
CompSt		:	LC DefList StmtList RC			{ $$ = createNode(4, "CompSt", $1, $2, $3, $4); }
			;
StmtList	:	Stmt StmtList					{ $$ = createNode(2, "StmtList", $1, $2); }
			|									{ $$ = NULL; }
			;
Stmt		:	Exp	SEMI						{ $$ = createNode(2, "Stmt", $1, $2); }
			|	CompSt							{ $$ = createNode(1, "Stmt", $1); }
			|	RETURN Exp SEMI					{ $$ = createNode(3, "Stmt", $1, $2, $3); }
			|	IF LP Exp RP Stmt %prec LOWER_THAN_ELSE	{ $$ = createNode(5, "Stmt", $1, $2, $3, $4, $5); }
			|	IF LP Exp RP Stmt ELSE Stmt		{ $$ = createNode(7, "Stmt", $1, $2, $3, $4, $5, $6, $7); }
			|	WHILE LP Exp RP Stmt			{ $$ = createNode(5, "Stmt", $1, $2, $3, $4, $5); }
			;
DefList		:	Def DefList						{ $$ = createNode(2, "DefList", $1, $2); }
			|									{ $$ = NULL; }
			;
Def			:	Specifier DecList SEMI			{ $$ = createNode(3, "Def", $1, $2, $3); }
			;
DecList		:	Dec								{ $$ = createNode(1, "DecList", $1); }
			|	Dec COMMA DecList				{ $$ = createNode(3, "DecList", $1, $2, $3); }
			;
Dec			:	VarDec							{ $$ = createNode(1, "Dec", $1); }
			|	VarDec ASSIGNOP Exp				{ $$ = createNode(3, "Dec", $1, $2, $3); }
			;
Exp			:	Exp ASSIGNOP Exp				{ $$ = createNode(3, "Exp", $1, $2, $3); }
			|	Exp AND Exp						{ $$ = createNode(3, "Exp", $1, $2, $3); }
			|	Exp OR Exp						{ $$ = createNode(3, "Exp", $1, $2, $3); }
			| 	Exp RELOP Exp					{ $$ = createNode(3, "Exp", $1, $2, $3); }
			|	Exp PLUS Exp					{ $$ = createNode(3, "Exp", $1, $2, $3); }
			|	Exp MINUS Exp					{ $$ = createNode(3, "Exp", $1, $2, $3); }
			|	Exp STAR Exp					{ $$ = createNode(3, "Exp", $1, $2, $3); }
			|	Exp DIV Exp						{ $$ = createNode(3, "Exp", $1, $2, $3); }
			|	LP Exp RP						{ $$ = createNode(3, "Exp", $1, $2, $3); }
			|	MINUS Exp						{ $$ = createNode(2, "Exp", $1, $2); }
			|	NOT Exp							{ $$ = createNode(2, "Exp", $1, $2); }
			|	ID LP Args RP					{ $$ = createNode(4, "Exp", $1, $2, $3, $4); }
			|	ID LP RP						{ $$ = createNode(3, "Exp", $1, $2, $3); }
			|	Exp LB Exp RB					{ $$ = createNode(4, "Exp", $1, $2, $3, $4); }
			|	Exp DOT ID						{ $$ = createNode(3, "Exp", $1, $2, $3); }	
			|	ID								{ $$ = createNode(1, "Exp", $1); }
			|	INT								{ $$ = createNode(1, "Exp", $1); }
			|	FLOAT							{ $$ = createNode(1, "Exp", $1); }
			;
Args		:	Exp COMMA Args					{ $$ = createNode(3, "Args", $1, $2, $3); }
			|	Exp								{ $$ = createNode(1, "Args", $1); }
			;

%%
#include "lex.yy.c"
int main(int argc, char** argv) {
	if (argc <= 1) return 1;
	FILE *f = fopen(argv[1], "r");
	if (!f) {
		perror(argv[1]); return 1;
	}
	yyrestart(f); yyparse();
	return 0;
}
yyerror(char* msg) {
	fprintf(stderr, "error: %s\n", msg);
}
