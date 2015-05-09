%{
#include "syntax.tab.h"
#include "node.h"
extern int isError;
%}

%option yylineno
digit		[0-9]
letter		[A-Za-z_]
float		{digit}*(\.{digit}*)([Ee][+-]?{digit}+)?
int			0|([1-9]{digit}{0,9})
octal		0[0-7]+
hex			0[xX][0-9A-Fa-f]+
id			({letter})({letter}|{digit})*
delim		[ \t]
ws			{delim}+
comment		\/\*([^*"]*|".*"|\*+[^/])*\*\/
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
"("			{ yylval.type_node = newNode(1, yylineno, "LP", yytext); return LP; }
")"			{ yylval.type_node = newNode(1, yylineno, "RP", yytext); return RP; }
"["			{ yylval.type_node = newNode(1, yylineno, "LB", yytext); return LB; }
"]"			{ yylval.type_node = newNode(1, yylineno, "RB", yytext); return RB; }
"{"			{ yylval.type_node = newNode(1, yylineno, "LC", yytext); return LC; }
"}"			{ yylval.type_node = newNode(1, yylineno, "RC", yytext); return RC; }
"//"		{ char c = input(); while (c != '\n') c = input(); }
{comment}	{}
int|float	{ yylval.type_node = newNode(1, yylineno, "TYPE", yytext); return TYPE; }
struct		{ yylval.type_node = newNode(1, yylineno, "STRUCT", yytext); return STRUCT; }
return		{ yylval.type_node = newNode(1, yylineno, "RETURN", yytext); return RETURN; }
if			{ yylval.type_node = newNode(1, yylineno, "IF", yytext); return IF;  }
else		{ yylval.type_node = newNode(1, yylineno, "ELSE", yytext); return ELSE; }
while		{ yylval.type_node = newNode(1, yylineno, "WHILE", yytext); return WHILE; }
{id}		{ yylval.type_node = newNode(1, yylineno, "ID", yytext); return ID; }
{float}		{ yylval.type_node = newNode(1, yylineno, "FLOAT", yytext); return FLOAT; }
{int}		{ yylval.type_node = newNode(1, yylineno, "INT", yytext); return INT; }
{octal}		{ yylval.type_node = newNode(1, yylineno, "OCTAL", yytext); return INT; }
{hex}		{ yylval.type_node = newNode(1, yylineno, "HEX", yytext); return INT; }
\n			{ }
.			{ isError = 1; printf("Error type A at Line %d: Mysterious characters \'%s\'\n", yylineno, yytext);}
%%

