#include <ctype.h>
#include "arcc.h"

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
    
    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';'){
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

    if(*p >= 'a' && *p <= 'z'){
      tokens[i].ty = TK_IDENT;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }
    
    error("cannot tokenize %c", p);
    exit(1);
  }

  tokens[i].ty = TK_EOF;
  tokens[i].input = p;
}
