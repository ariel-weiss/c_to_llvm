#include "tokens.hpp"
#include <stdio.h>
#include <string.h>	
#include <stdlib.h>
#include <stack> 

const char* token_to_str(int token);
int doOp(const char* op, int a, int b);

int main()
{

	std::stack<std::pair<const char*,bool>> _input;
	std::stack<int> _calc;
	char* buff;

	char* test;
	int token;
	while(token = yylex()) {
		if (token == NUM) {
			buff = (char*)malloc(strlen(yytext)+1);
			strcpy(buff,yytext);
			auto item = std::make_pair(buff, true);
			_input.push(item);
		
		}
		else if (token == BINOP) {
			buff = (char*)malloc(strlen(yytext)+1);
			strcpy(buff,yytext);
			auto item = std::make_pair(buff, false);
			_input.push(item);
			
		}
		else if(token == -2){ //Not a token
			printf("Error %s\n",yytext);
			return 0;
		}
		else { //Invalid token
			printf("Error: %s\n",token_to_str(token));
			return 0;
		}
	}


	//All items in stack, compute expr:
	std::pair<const char*, bool> cur;
	int a, b, res;
	
	while (!_input.empty()) {
		cur = _input.top();
		_input.pop();
		if (cur.second) { // True == NUM
			_calc.push(atoi(cur.first));
		}
		else 
		{
			if (_calc.size() < 2) {
				printf("Error: Bad Expression\n");
				return 0;
				} 
			a = _calc.top();
			_calc.pop();
			//Check if not last??
			b = _calc.top();
			_calc.pop();
			res = doOp(cur.first, a, b);
			_calc.push(res);

		}
		free((char*)cur.first);
	}
	if(_calc.size() != 1)
		printf("Error: Bad Expression\n");
	else
		printf("%d\n",_calc.top());
	return 0;
}

const char* token_to_str(int token){
	////might be better- CAPS yytext.......
	switch(token){
		case -1: return "STRING_ERROR";
		case VOID: return "VOID"; 
		case INT: return "INT"; 
		case BYTE: return "BYTE"; 
		case B: return "B"; 
		case BOOL: return "BOOL"; 
		case AND: return "AND"; 
		case OR: return "OR";
		case NOT: return "NOT"; 
		case TRUE: return "TRUE"; 
		case FALSE: return "FALSE"; 
		case RETURN: return "RETURN"; 
		case IF: return "IF"; 
		case ELSE: return "ELSE"; 
		case WHILE: return "WHILE"; 
		case BREAK: return "BREAK"; 
		case CONTINUE: return "CONTINUE"; 
		case SC: return "SC"; 
		case COMMA: return "COMMA"; 
		case LPAREN: return "LPAREN"; 
		case RPAREN: return "RPAREN"; 
		case LBRACE: return "LBRACE"; 
		case RBRACE: return "RBRACE";
		case ASSIGN: return "ASSIGN"; 
		case RELOP: return "RELOP"; 
		case BINOP: return "BINOP"; 
		case COMMENT: return "COMMENT";
		case ID: return "ID";
		case NUM: return "NUM"; 
		case STRING: return "STRING"; 
		default: return "ERROR";
	}
}


int doOp(const char* op, int a, int b) {
	if (!strcmp(op, "-")) {
		return a - b;
	}
	if (!strcmp(op, "+")) {
		return a + b;
	}
	if (!strcmp(op, "/")) {
		return a / b;
	}
	if (!strcmp(op,"*")) {
		return a * b;
	}
	return 0;
}

