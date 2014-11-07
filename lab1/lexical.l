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
";"			{ yylval.type_node = node("SEMI", yytext); return SEMI; }
","			{ yylval.type_node = node("COMMA", yytext); return COMMA; }
"="			{ yylval.type_node = node("ASSIGNOP", yytext); return ASSIGNOP; }
">"			{ yylval.type_node = node("GT", yytext); return RELOP; }
"<"     	{ yylval.type_node = node("LT", yytext); return RELOP; }
">="		{ yylval.type_node = node("GE", yytext); return RELOP; }
"<="		{ yylval.type_node = node("LE", yytext); return RELOP; }
"=="		{ yylval.type_node = node("EQ", yytext); return RELOP; }
"!="		{ yylval.type_node = node("NQ", yytext); return RELOP; }
"+"			{ yylval.type_node = node("PLUS", yytext); return PLUS; }
"-"			{ yylval.type_node = node("MINUS", yytext); return MINUS; }
"*"			{ yylval.type_node = node("STAR", yytext); return STAR; }
"/"			{ yylval.type_node = node("DIV", yytext); return DIV; }
"&&"		{ yylval.type_node = node("AND", yytext); return AND; }
"||"		{ yylval.type_node = node("OR", yytext); return OR; }
"."			{ yylval.type_node = node("DOT", yytext); return DOT; }
"!"			{ yylval.type_node = node("NOT", yytext); return NOT; }
int|float	{ yylval.type_node = node("TYPE", yytext); return TYPE; }
"("			{ yylval.type_node = node("LP", yytext); return LP; }
")"			{ yylval.type_node = node("RP", yytext); return RP; }
"["			{ yylval.type_node = node("LB", yytext); return LB; }
"]"			{ yylval.type_node = node("RB", yytext); return RB; }
"{"			{ yylval.type_node = node("LC", yytext); return LC; }
"}"			{ yylval.type_node = node("RC", yytext); return RC; }
struct		{ yylval.type_node = node("STRUCT", yytext); return STRUCT; }
return		{ yylval.type_node = node("RETURN", yytext); return RETURN; }
if			{ yylval.type_node = node("IF", yytext); return IF;  }
else		{ yylval.type_node = node("ELSE", yytext); return ELSE; }
while		{ yylval.type_node = node("WHILE", yytext); return WHILE; }
{id}		{ yylval.type_node = node("ID", yytext); return ID; }
{float}		{ yylval.type_node = node("FLOAT", yytext); return FLOAT; }
{int}		{ yylval.type_node = node("INT", yytext); return INT; }
.			{ printf("Error type A at Line %d: Mysterious characters \'%s\'\n",
				 yylineno, yytext);
			}
%%

