
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "arcc.h"

static bool is_valid_leading(char ch){
  return (ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z') || (ch == '_');  
}

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

char *strndup(const char *s, size_t n) {
    char *p = memchr(s, '\0', n);
    if (p != NULL)
        n = p - s;
    p = malloc(n + 1);
    if (p != NULL) {
        memcpy(p, s, n);
        p[n] = '\0';
    }
    return p;
}

static Token *new_token_ident(char *ident, char *input){
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

    if(*p == '|' && *(p+1) == '|'){
      push_back(tokens, new_token(TK_OR, "||", 0));
      p+=2;
      continue;
    }

    if(*p == '&' && *(p+1) == '&'){
      push_back(tokens, new_token(TK_AND, "&&", 0));
      p+=2;
      continue;
    }

    if(*p == '+' && *(p+1) == '+'){
      push_back(tokens, new_token(TK_INC, "++", 0));
      p+=2;
      continue;
    }

    if(*p == '+' && *(p+1) == '+'){
      push_back(tokens, new_token(TK_DEC, "++", 0));
      p+=2;
      continue;
    }

    if(*p == '+' && *(p+1) == '='){
      push_back(tokens, new_token(TK_PLUS_EQ, "+=", 0));
      p+=2;
      continue;
    }

    if(*p == '-' && *(p+1) == '='){
      push_back(tokens, new_token(TK_MINUS_EQ, "-=", 0));
      p+=2;
      continue;
    }

    if(*p == '*' && *(p+1) == '='){
      push_back(tokens, new_token(TK_MUL_EQ, "*=", 0));
      p+=2;
      continue;
    }

    if(*p == '/' && *(p+1) == '='){
      push_back(tokens, new_token(TK_DIV_EQ, "/=", 0));
      p+=2;
      continue;
    }

    
    
    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=' || *p == '{' || *p == '}'){
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

    if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])){
      push_back(tokens, new_token(TK_IF, p, 0));
      p+=2;
      continue;
    }

    if(strncmp(p, "else if", 7) == 0 && !is_alnum(p[7])){
      push_back(tokens, new_token(TK_ELSE_IF, p, 0));
      p+=7;
      continue;
    }
    
    if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])){
      push_back(tokens, new_token(TK_ELSE, p, 0));
      p+=4;
      continue;
    }

    if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])){
      push_back(tokens, new_token(TK_WHILE, p, 0));
      p+=5;
      continue;
    }    

    if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])){
      push_back(tokens, new_token(TK_FOR, p, 0));
      p+=3;
      continue;
    }    
    
    if(is_valid_leading(*p)){
      int len = 1;
      while(is_alnum(*(p+len))){
        len++;
      }
      char *ident = strndup(p, len);
      push_back(tokens, new_token_ident(ident, p));
      p+=len;
      continue;
    }
    
    error("cannot tokenize %s", p);
    exit(1);
  }

  push_back(tokens, new_token(TK_EOF, p, 0));
  return tokens;
}
