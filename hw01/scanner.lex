%{

#include<stdio.h>
#include<string.h>	
#include<stdlib.h>
#include "tokens.hpp"

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



\/[\/]+.*																						return COMMENT;

";"																								return SC;
","																							    return COMMA;
"{"																								return LBRACE;
"}"																								return RBRACE;
"("																								return LPAREN;
")"																								return RPAREN;
"="																								return ASSIGN;
"=="|"!="|">"|"<"|"<="|">="                                                                     return RELOP;
"+"|"-"|"/"|"*"                                                                                 return BINOP;


"void"																							return VOID;
"int"																							return INT;
"byte"																							return BYTE;
"b"																								return B;
"bool"																							return BOOL;
"and"																							return AND;
"or"																							return OR;
"not"																							return NOT;
"true"																							return TRUE;
"false"																							return FALSE;
"return"																						return RETURN;
"if"																							return IF;
"else"																							return ELSE;
"while"																							return WHILE;
"break"																							return BREAK;
"continue"																						return CONTINUE;


(0)                                                                           			    	return NUM;
({digit_no_zero}{digit}*)																	    return NUM;
({letter}{id_char}*)																			return ID;


L?\"(\\.|[^\\"])*\"   																			return STRING;
L?\"(\\.|[^\\"])*   																			return -1;


{whitespace}																					;
.																								return -2;

%%




