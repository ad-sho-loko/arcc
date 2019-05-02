#include <ctype.h>
#include "arcc.h"

static Token *new_token(int ty, char* input, int val){
  Token *t = malloc(sizeof(Token));
  t->ty = ty;
  t->input = input;
  t->val = val;
  return t;
}

static Token *new_token_ident(char ident, char* input){
  Token *t = malloc(sizeof(Token));
  t->ty = TK_IDENT;
  t->input = input;
  t->val = 0;
  t->name = ident;
  return t;
}


Vector *tokenize(char *p){
  Vector* tokens = new_vector();
  int i = 0;

  while(*p){

    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p == '=' && *(p+1) == '='){
      push_back(tokens, new_token(TK_EQL, "==", 0));
      i++;
      p+=2;
      continue;
    }

    if(*p == '!' && *(p+1) == '='){
      push_back(tokens, new_token(TK_NEQ, "!=", 0));
      i++;
      p+=2;
      continue;      
    }

    if(*p == '<' && *(p+1) == '='){
      push_back(tokens, new_token(TK_LE, "<=", 0));
      i++;
      p+=2;
      continue;      
    }

    if(*p == '>' && *(p+1) == '='){
      push_back(tokens, new_token(TK_GE, ">=", 0));
      i++;
      p+=2;
      continue;      
    }
    
    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '='){
      push_back(tokens, new_token(*p, p, 0));
      i++;
      p++;
      continue;
    }

    if(isdigit(*p)){
      push_back(tokens, new_token(TK_NUM, p, strtol(p, &p ,10)));
      i++;
      continue;
    }

    if(*p >= 'a' && *p <= 'z'){
      push_back(tokens, new_token_ident(*p, p));
      i++;
      p++;
      continue;
    }
    
    error("cannot tokenize %c", p);
    exit(1);
  }

  push_back(tokens, new_token(TK_EOF, p, 0));
  return tokens;
}
