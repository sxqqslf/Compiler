%{
	#include "lex.yy.c"
	#include "node.h"
	int isError;
	struct node *root;
%}

%error-verbose

%union {
	int type_int;
	float type_float;
	double type_double;
	struct node *type_node;
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
ExtDefList	:	ExtDef ExtDefList				{ $$ = createNode(2, "ExtDefList", $2, $1); }
			|									{ $$ = NULL; }
			;
ExtDef		:	Specifier ExtDecList SEMI		{ $$ = createNode(3, "ExtDef", $3, $2, $1); }
			|	Specifier SEMI					{ $$ = createNode(2, "ExtDef", $2, $1); }
			|	Specifier FunDec CompSt			{ $$ = createNode(3, "ExtDef", $3, $2, $1); }
			|	error SEMI						{ isError = 1; }
			;
ExtDecList	:	VarDec							{ $$ = createNode(1, "ExtDecList", $1); }
			|	VarDec COMMA ExtDecList			{ $$ = createNode(3, "ExtDecList", $3, $2, $1); }
			;
Specifier	:	TYPE 							{ $$ = createNode(1, "Specifier", $1); }
			|	StructSpecifier					{ $$ = createNode(1, "Specifier", $1); }
			;
StructSpecifier	:	STRUCT OptTag LC DefList RC	{ $$ = createNode(5, "StructSpecifier", $5, $4, $3, $2, $1); }
				|	STRUCT Tag					{ $$ = createNode(2, "StructSpecifier", $2, $1); }
				;
OptTag		:	ID								{ $$ = createNode(1, "OptTag", $1); }
			|									{ $$ = NULL; }
			;
Tag			:	ID								{ $$ = createNode(1, "Tag", $1); }
			;					
VarDec		:	ID								{ $$ = createNode(1, "VarDec", $1); }
			|	VarDec LB INT RB				{ $$ = createNode(4, "VarDec", $4, $3, $2, $1); }
			;
FunDec		:	ID LP VarList RP				{ $$ = createNode(4, "FunDec", $4, $3, $2, $1); }
			|	ID LP RP						{ $$ = createNode(3, "FunDec", $3, $2, $1); }
			|	error RP						{ isError = 1; }
			;
VarList		:	ParamDec COMMA VarList			{ $$ = createNode(3, "VarList", $3, $2, $1); }
			|	ParamDec						{ $$ = createNode(1, "VarList", $1); }
			;
ParamDec	:	Specifier VarDec				{ $$ = createNode(2, "ParamDec", $2, $1); }
			;
CompSt		:	LC DefList StmtList RC			{ $$ = createNode(4, "CompSt", $4, $3, $2, $1); }
			|	error RC						{ isError = 1; }
			;
StmtList	:	Stmt StmtList					{ $$ = createNode(2, "StmtList", $2, $1); }
			|									{ $$ = NULL; }
			;
Stmt		:	Exp	SEMI						{ $$ = createNode(2, "Stmt", $2, $1); }
			|	CompSt							{ $$ = createNode(1, "Stmt", $1); }
			|	RETURN Exp SEMI					{ $$ = createNode(3, "Stmt", $3, $2, $1); }
			|	IF LP Exp RP Stmt %prec LOWER_THAN_ELSE	{ $$ = createNode(5, "Stmt", $5, $4, $3, $2, $1); }
			|	IF LP Exp RP Stmt ELSE Stmt		{ $$ = createNode(7, "Stmt", $7, $6, $5, $4, $3, $2, $1); }
			|	WHILE LP Exp RP Stmt			{ $$ = createNode(5, "Stmt", $5, $4, $3, $2, $1); }
			|	error RP						{ isError = 1; }
			|	error SEMI						{ isError = 1; }
			;
DefList		:	Def DefList						{ $$ = createNode(2, "DefList", $2, $1); }
			|									{ $$ = NULL; }
			;
Def			:	Specifier DecList SEMI			{ $$ = createNode(3, "Def", $3, $2, $1); }
			|	error SEMI						{ isError = 1; }
			;
DecList		:	Dec								{ $$ = createNode(1, "DecList", $1); }
			|	Dec COMMA DecList				{ $$ = createNode(3, "DecList", $3, $2, $1); }
			;
Dec			:	VarDec							{ $$ = createNode(1, "Dec", $1); }
			|	VarDec ASSIGNOP Exp				{ $$ = createNode(3, "Dec", $3, $2, $1); }
			;
Exp			:	Exp ASSIGNOP Exp				{ $$ = createNode(3, "Exp", $3, $2, $1); }
			|	Exp AND Exp						{ $$ = createNode(3, "Exp", $3, $2, $1); }
			|	Exp OR Exp						{ $$ = createNode(3, "Exp", $3, $2, $1); }
			| 	Exp RELOP Exp					{ $$ = createNode(3, "Exp", $3, $2, $1); }
			|	Exp PLUS Exp					{ $$ = createNode(3, "Exp", $3, $2, $1); }
			|	Exp MINUS Exp					{ $$ = createNode(3, "Exp", $3, $2, $1); }
			|	Exp STAR Exp					{ $$ = createNode(3, "Exp", $3, $2, $1); }
			|	Exp DIV Exp						{ $$ = createNode(3, "Exp", $3, $2, $1); }
			|	LP Exp RP						{ $$ = createNode(3, "Exp", $3, $2, $1); }
			|	MINUS Exp						{ $$ = createNode(2, "Exp", $2, $1); }
			|	NOT Exp							{ $$ = createNode(2, "Exp", $2, $1); }
			|	ID LP Args RP					{ $$ = createNode(4, "Exp", $4, $3, $2, $1); }
			|	ID LP RP						{ $$ = createNode(3, "Exp", $3, $2, $1); }
			|	Exp LB Exp RB					{ $$ = createNode(4, "Exp", $4, $3, $2, $1); }
			|	Exp DOT ID						{ $$ = createNode(3, "Exp", $3, $2, $1); }	
			|	ID								{ $$ = createNode(1, "Exp", $1); }
			|	INT								{ $$ = createNode(1, "Exp", $1); }
			|	FLOAT							{ $$ = createNode(1, "Exp", $1); }
			|	Exp LB Exp error SEMI			{ isError = 1; } 
			;
Args		:	Exp COMMA Args					{ $$ = createNode(3, "Args", $3, $2, $1); }
			|	Exp								{ $$ = createNode(1, "Args", $1); }
			;

%%
int main(int argc, char** argv) {
	FILE *f = fopen(argv[1], "r");
	if (!f) {
		perror(argv[1]); return 1;
	}
	root = NULL; isError = 0;
	yyrestart(f); yyparse();
	if (!isError)
		printTree(root, 0);
	return 0;
}
yyerror(char* msg) {
	printf("Error type B at Line %d: %s.\n", yylineno, msg);
}
