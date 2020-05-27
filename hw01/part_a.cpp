#include "tokens.hpp"
#include <stdio.h>
#include <string.h>	
#include <stdlib.h>
#include <ctype.h>

bool printString(const char* text);
void showToken(const char* token);
const char* token_to_str(int token);
char compress(char c);
long getHex(char first,char second);

int main()
{
	int token;
	while(token = yylex()) {
	// Your code here
		showToken(token_to_str(token));
		//printf("TOKEN[%d]-->[%s]\n",token,token_to_str(token));
	}
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

void showToken(const char* token){
	if(!strcmp(token,"ERROR")){
		//Not matching token
		printf("Error %s\n",yytext);
		exit(0);
	}
	else if(!strcmp(token,"COMMENT")){
		printf("%d COMMENT //\n",yylineno);
	}
	else if(!strcmp(token,"STRING")){
		if(!printString(yytext)) 
			exit(0);
	}
	else if(!strcmp(token,"STRING_ERROR")){
		printf("Error unclosed string\n");
		exit(0);
	}
	else{
		printf("%d %s %s\n",yylineno,token,yytext);
	}
}

bool printString(const char* text){
	char buffer[1024];
	int cur = 0;
	int len = yyleng;//=strlen(yytext)
	char c;
	for(int i = 1; i < len - 1; i++){
		if(text[i] == '\n'){
			printf("Error unclosed string\n"); //ERRORROROROROR!!!
			return false;
		}
		if(text[i] == '\\'){             ///// \xdd df
			if(i < len - 2 && text[i+1] == 'x'){
				long hex_res = getHex(text[i+2],text[i+3]);
				if(hex_res != -1){
					buffer[cur++] = (char)hex_res;
				}else{
					if(text[i+2] == '"')
						printf("Error undefined escape sequence x\n"); 
					else if(text[i+3] == '"')
						printf("Error undefined escape sequence x%c\n",text[i+2]); 
					else
						printf("Error undefined escape sequence x%c%c\n",text[i+2],text[i+3]); 
					return false;
				}
				i+=3;
				continue;
			}
			c = compress(text[i+1]);
			if (c != '1'){
				buffer[cur++] = c;
			}
			else{
				printf("Error undefined escape sequence %c\n",text[i+1]); //ERRORROROROROR!!!
				return false;
			}
			i++;
			continue;
		}
		buffer[cur++] = text[i];
	}
	
	//Last two separately
	
	if(text[strlen(text) - 1] == '\\'){
		c = compress(text[strlen(text)]);
		if (c != '1'){
				buffer[cur++] = c;
		}
		else{
				printf("Error undefined escape sequence %c\n",text[strlen(text)]); //ERRORROROROROR!!!
				return false;
			}
	}
	
	buffer[cur] = '\0';
	printf("%d STRING %s\n",yylineno,buffer);
	return true;
}

char compress(char c){
	
			if(c == 'n')
				return '\n';
			if(c == 't')
				return '\t';
			if(c == 'r')
				return '\r';
			if(c == '0')
				return '\0';
			if(c == '"')
				return '\"';
			if(c == '\\')
				return '\\';
			
			return '1';
}

long getHex(char first,char second){
	char buff[3];
	//Check If Valid
	if(!isalnum(first) || !isalnum(second)) return -1;
	if(first == '0' && second == '0') return 0;
    first = toupper(first); second = toupper(second);
	if((first > 'F' || first < '0') || (second > 'F' || second < '0')) return -1;
	
	//Insert to buffer
	buff[0] = first;
	buff[1] = second;
	buff[2] = '\0';
	
	long num = strtol(buff, NULL, 16); 
	if( num == 0 || num < 0x00 || num > 0x7F){
		//Invalid
		return -1;
	}
	return num;
	
}
