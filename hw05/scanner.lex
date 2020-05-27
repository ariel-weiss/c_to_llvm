%{

#include "parser.h"
#include "parser.tab.hpp"
#include "hw3_output.hpp"

void lexError(int n);

%}

%option yylineno
%option noyywrap

digit   		([0-9])
digit_no_zero	([1-9])
letter  		([A-Za-z])
id_char         ([A-Za-z0-9])
whitespace		[\t\n\r ]
printable	    ([\x20-\x7E]|0x09]|[0x0A]|[0x0D]) 
Hex				([A-Fa-f0-9])
	

%%




";"																								return SC;
","																							    return COMMA;
"{"																								return LBRACE;
"}"																								return RBRACE;
"("																								return LPAREN;
")"																								return RPAREN;
"="																								return ASSIGN;
"=="|"!="																						{yylval = new STEntry(yytext);return RELOPB;}
">"|"<"|"<="|">="                                                                               {yylval = new STEntry(yytext);return RELOPA;}
"+"|"-"                                                                                         {yylval = new STEntry(yytext);return BINOPB;}
"*"|"/"                                                                                         {yylval = new STEntry(yytext);return BINOPA;}

"void"																							return VOID;
"int"																							return INT;
"byte"																							return BYTE;
"b"																								return B;
"bool"																							return BOOL;
"enum"																							return ENUM;
"and"																							return AND;
"or"																							return OR;
"not"																							return NOT;
"true"																							{yylval = new STEntry("TRUE","BOOL"); return TRUE;}
"false"																							{yylval = new STEntry("FALSE","BOOL"); return FALSE;}
"return"																						return RETURN;
"if"																							return IF;
"else"																							return ELSE;
"while"																							return WHILE;
"break"																							return BREAK;
"continue"																						return CONTINUE;


(0)                                                                           			    	{yylval = new Num("INT",yytext); return NUM;}
({digit_no_zero}{digit}*)																	    {yylval = new Num("INT",yytext); return NUM;}
({letter}{id_char}*)																			{yylval = new STEntry(yytext,yytext); return ID;}


L?\"(\\.|[^\\"])*\"   																			{yylval = new STEntry(yytext,"STRING"); return STRING;}
L?\"(\\.|[^\\"])*   																			lexError(yylineno);

\/[\/]+.*																						;
{whitespace}																					;
.																								lexError(yylineno);

%%

void lexError(int n){
	output::errorLex(n);
	exit(0);
}


