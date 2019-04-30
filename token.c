#include <ctype.h>
#include "arcc.h"

Token tokens[100];

void tokenize(char *p){
  int i = 0;
  while(*p){

    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p == '=' && *(p+1) == '='){
      tokens[i].ty = TK_EQL;
      tokens[i].input = "==";
      i++;
      p+=2;
      continue;
    }

    if(*p == '!' && *(p+1) == '='){
      tokens[i].ty = TK_NEQ;
      tokens[i].input = "!=";
      i++;
      p+=2;
      continue;      
    }

    if(*p == '<' && *(p+1) == '='){
      tokens[i].ty = TK_LE;
      tokens[i].input = "<=";
      i++;
      p+=2;
      continue;      
    }

    if(*p == '>' && *(p+1) == '='){
      tokens[i].ty = TK_GE;
      tokens[i].input = ">=";
      i++;
      p+=2;
      continue;      
    }
    
    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>'){
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if(isdigit(*p)){
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p ,10);
      i++;
      continue;
    }

    error("cannot tokenize char:`%c`", p);
    exit(1);
  }

  tokens[i].ty = TK_NUM;
  tokens[i].input = p;
}
