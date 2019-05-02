#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "arcc.h"

static bool is_alnum(char ch){
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch >= '0'&& ch <= '9') || (ch == '_');
}

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

  while(*p){

    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p == '=' && *(p+1) == '='){
      push_back(tokens, new_token(TK_EQL, "==", 0));
      p+=2;
      continue;
    }

    if(*p == '!' && *(p+1) == '='){
      push_back(tokens, new_token(TK_NEQ, "!=", 0));
      p+=2;
      continue;      
    }

    if(*p == '<' && *(p+1) == '='){
      push_back(tokens, new_token(TK_LE, "<=", 0));
      p+=2;
      continue;      
    }

    if(*p == '>' && *(p+1) == '='){
      push_back(tokens, new_token(TK_GE, ">=", 0));
      p+=2;
      continue;      
    }
    
    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '='){
      push_back(tokens, new_token(*p, p, 0));
      p++;
      continue;
    }

    if(isdigit(*p)){
      push_back(tokens, new_token(TK_NUM, p, strtol(p, &p ,10)));
      continue;
    }

    if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])){
      push_back(tokens, new_token(TK_RETURN, p, 0));
      p+=6;
      continue;
    }

    if(*p >= 'a' && *p <= 'z'){
      push_back(tokens, new_token_ident(*p, p));
      p++;
      continue;
    }

    error("cannot tokenize %c", p);
    exit(1);
  }

  push_back(tokens, new_token(TK_EOF, p, 0));
  return tokens;
}
