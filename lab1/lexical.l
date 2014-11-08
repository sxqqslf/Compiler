%{
	#include "syntax.tab.h"
	#include "node.h"
%}

%option yylineno
digit		[0-9]
letter		[A-Za-z]
float		{digit}+(\.{digit}+)
int			0|([1-9]{digit}{0,9})
id			({letter}|_)({letter}|{digit})*
delim		[ \t\n]
ws			{delim}+
%%
{ws}		{ /*no thing*/ }
";"			{ yylval.type_node = newNode(1, yylineno, "SEMI", yytext); return SEMI; }
","			{ yylval.type_node = newNode(1, yylineno, "COMMA", yytext); return COMMA; }
"="			{ yylval.type_node = newNode(1, yylineno, "ASSIGNOP", yytext); return ASSIGNOP; }
">"			{ yylval.type_node = newNode(1, yylineno, "GT", yytext); return RELOP; }
"<"     	{ yylval.type_node = newNode(1, yylineno, "LT", yytext); return RELOP; }
">="		{ yylval.type_node = newNode(1, yylineno, "GE", yytext); return RELOP; }
"<="		{ yylval.type_node = newNode(1, yylineno, "LE", yytext); return RELOP; }
"=="		{ yylval.type_node = newNode(1, yylineno, "EQ", yytext); return RELOP; }
"!="		{ yylval.type_node = newNode(1, yylineno, "NQ", yytext); return RELOP; }
"+"			{ yylval.type_node = newNode(1, yylineno, "PLUS", yytext); return PLUS; }
"-"			{ yylval.type_node = newNode(1, yylineno, "MINUS", yytext); return MINUS; }
"*"			{ yylval.type_node = newNode(1, yylineno, "STAR", yytext); return STAR; }
"/"			{ yylval.type_node = newNode(1, yylineno, "DIV", yytext); return DIV; }
"&&"		{ yylval.type_node = newNode(1, yylineno, "AND", yytext); return AND; }
"||"		{ yylval.type_node = newNode(1, yylineno, "OR", yytext); return OR; }
"."			{ yylval.type_node = newNode(1, yylineno, "DOT", yytext); return DOT; }
"!"			{ yylval.type_node = newNode(1, yylineno, "NOT", yytext); return NOT; }
int|float	{ yylval.type_node = newNode(1, yylineno, "TYPE", yytext); return TYPE; }
"("			{ yylval.type_node = newNode(1, yylineno, "LP", yytext); return LP; }
")"			{ yylval.type_node = newNode(1, yylineno, "RP", yytext); return RP; }
"["			{ yylval.type_node = newNode(1, yylineno, "LB", yytext); return LB; }
"]"			{ yylval.type_node = newNode(1, yylineno, "RB", yytext); return RB; }
"{"			{ yylval.type_node = newNode(1, yylineno, "LC", yytext); return LC; }
"}"			{ yylval.type_node = newNode(1, yylineno, "RC", yytext); return RC; }
struct		{ yylval.type_node = newNode(1, yylineno, "STRUCT", yytext); return STRUCT; }
return		{ yylval.type_node = newNode(1, yylineno, "RETURN", yytext); return RETURN; }
if			{ yylval.type_node = newNode(1, yylineno, "IF", yytext); return IF;  }
else		{ yylval.type_node = newNode(1, yylineno, "ELSE", yytext); return ELSE; }
while		{ yylval.type_node = newNode(1, yylineno, "WHILE", yytext); return WHILE; }
{id}		{ yylval.type_node = newNode(1, yylineno, "ID", yytext); return ID; }
{float}		{ yylval.type_node = newNode(1, yylineno, "FLOAT", yytext); return FLOAT; }
{int}		{ yylval.type_node = newNode(1, yylineno, "INT", yytext); return INT; }
.			{ printf("Error type A at Line %d: Mysterious characters \'%s\'\n",
				 yylineno, yytext);
			}
%%

