#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "arcc.h"

Vector *tokens;

static bool valid_leading(char ch){
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
  t->name = NULL;
  return t;
}

static Token *new_token_string(char *input, char* string){
  Token *t = malloc(sizeof(Token));
  t->ty = TK_STRING;
  t->name = NULL;
  t->input = input;
  t->string = string;
  return t;
}

static Token *new_token_op(int ty, char* input){
  Token *t = malloc(sizeof(Token));
  t->ty = ty;
  t->input = input;
  t->val = 0;
  t->name = NULL;
  return t;
}

static Token *new_token_reserved(int ty, char* input){
  Token *t = malloc(sizeof(Token));
  t->ty = ty;
  t->input = input;
  t->val = 0;
  t->name = NULL;
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

static Token *new_token_type(int type, char* input){
  Token *t = malloc(sizeof(Token));
  t->ty = TK_TYPE;
  t->input = input;
  t->val = 0;
  t->name = NULL;
  t->type = malloc(sizeof(Type));
  t->type->ty = type;
  return t;
}

static Token *new_token_ident(char *ident, char *input){
  Token *t = malloc(sizeof(Token));
  t->ty = TK_IDENT;
  t->input = input;
  t->val = 0;
  t->name = ident;
  return t;
}

static bool keyword(char* p, char* word){
  int len = strlen(word);
  return strncmp(p, word, len) == 0 && !is_alnum(p[len]);
}

Vector *tokenize(char *p){
  tokens = new_vector();

  while(*p){

    if(isspace(*p)){
      p++;
      continue;
    }

    if(*p == '=' && *(p+1) == '='){
      push_back(tokens, new_token_op(TK_EQL, "=="));
      p+=2;
      continue;
    }

    if(*p == '!' && *(p+1) == '='){
      push_back(tokens, new_token_op(TK_NEQ, "!="));
      p+=2;
      continue;      
    }

    if(*p == '<' && *(p+1) == '='){
      push_back(tokens, new_token_op(TK_LE, "<="));
      p+=2;
      continue;      
    }

    if(*p == '>' && *(p+1) == '='){
      push_back(tokens, new_token_op(TK_GE, ">="));
      p+=2;
      continue;      
    }

    if(*p == '<' && *(p+1) == '<' && *(p+2) == '='){
      push_back(tokens, new_token_op(TK_LSHIFT_EQ, "<<="));
      p+=3;
      continue;
    }    

    if(*p == '>' && *(p+1) == '>' && *(p+2) == '='){
      push_back(tokens, new_token_op(TK_RSHIFT_EQ, "<<="));
      p+=3;
      continue;
    }    

    if(*p == '>' && *(p+1) == '>'){
      push_back(tokens, new_token_op(TK_RSHIFT, ">>"));
      p+=2;
      continue;      
    }

    if(*p == '<' && *(p+1) == '<'){
      push_back(tokens, new_token_op(TK_LSHIFT, "<<"));
      p+=2;
      continue;      
    }
    
    if(*p == '|' && *(p+1) == '|'){
      push_back(tokens, new_token_op(TK_OR, "||"));
      p+=2;
      continue;
    }

    if(*p == '&' && *(p+1) == '&'){
      push_back(tokens, new_token_op(TK_AND, "&&"));
      p+=2;
      continue;
    }

    if(*p == '+' && *(p+1) == '+'){
      push_back(tokens, new_token_op(TK_INC, "++"));
      p+=2;
      continue;
    }

    if(*p == '-' && *(p+1) == '-'){
      push_back(tokens, new_token_op(TK_DEC, "--"));
      p+=2;
      continue;
    }

    if(*p == '+' && *(p+1) == '='){
      push_back(tokens, new_token_op(TK_PLUS_EQ, "+="));
      p+=2;
      continue;
    }

    if(*p == '-' && *(p+1) == '='){
      push_back(tokens, new_token_op(TK_MINUS_EQ, "-="));
      p+=2;
      continue;
    }

    if(*p == '*' && *(p+1) == '='){
      push_back(tokens, new_token(TK_MUL_EQ, "*=", 0));
      p+=2;
      continue;
    }

    if(*p == '/' && *(p+1) == '='){
      push_back(tokens, new_token_op(TK_DIV_EQ, "/="));
      p+=2;
      continue;
    }
    
    if(*p == '%' && *(p+1) == '='){
      push_back(tokens, new_token_op(TK_REM_EQ, "%="));
      p+=2;
      continue;
    }    

    if(*p == '&' && *(p+1) == '='){
      push_back(tokens, new_token_op(TK_AND_EQ, "&="));
      p+=2;
      continue;
    }    
    
    if(*p == '|' && *(p+1) == '='){
      push_back(tokens, new_token_op(TK_OR_EQ, "|="));
      p+=2;
      continue;
    }    

    if(*p == '^' && *(p+1) == '='){
      push_back(tokens, new_token_op(TK_XOR_EQ, "^="));
      p+=2;
      continue;
    }    
    
    if(*p == '\'' && *(p+2) == '\''){
      push_back(tokens, new_token(TK_CHAR, p, *(p+1)));
      p+=3;
      continue;
    }

    if(*p == '"'){
      int i = 0;
      char buf[32];
      p++;
      while(*p != '"'){
        buf[i++]  = *p++;
      }
      p++;
      buf[i] = '\0';
      push_back(tokens, new_token_string(p, strndup(buf, i)));
      continue;
    }

    
    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=' || *p == '{' || *p == '}' || *p == ',' || *p == '%' || *p == '&' || *p == '|' || *p == '^' || *p == '~' || *p == '?' || *p == ':' || *p == '[' || *p == ']'){
      push_back(tokens, new_token(*p, p, 0));
      p++;
      continue;
    }

    if(isdigit(*p)){
      push_back(tokens, new_token(TK_NUM, p, strtol(p, &p ,10)));
      continue;
    }

    if(keyword(p, "return")){
      push_back(tokens, new_token_reserved(TK_RETURN, p));
      p+=6;
      continue;
    }

    if(keyword(p, "if")){
      push_back(tokens, new_token_reserved(TK_IF, p));
      p+=2;
      continue;
    }

    if(keyword(p, "else if")){
      push_back(tokens, new_token_reserved(TK_ELSE_IF, p));
      p+=7;
      continue;
    }
    
    if(keyword(p, "else")){
      push_back(tokens, new_token_reserved(TK_ELSE, p));
      p+=4;
      continue;
    }

    if(keyword(p, "while")){
      push_back(tokens, new_token_reserved(TK_WHILE, p));
      p+=5;
      continue;
    }    

    if(keyword(p, "for")){
      push_back(tokens, new_token_reserved(TK_FOR, p));
      p+=3;
      continue;
    }    

    if(keyword(p, "break")){
      push_back(tokens, new_token_reserved(TK_BREAK, p));
      p+=5;
      continue;
    }    

    if(keyword(p, "continue")){
      push_back(tokens, new_token_reserved(TK_CONTINUE, p));
      p+=8;
      continue;
    }    

    if(keyword(p, "int")){
      push_back(tokens, new_token_type(INT, p));
      p+=3;
      continue;
    }

    if(keyword(p, "char")){
      push_back(tokens, new_token_type(CHAR, p));
      p+=4;
      continue;
    }    
    
    if(keyword(p, "sizeof")){
      push_back(tokens, new_token_reserved(TK_SIZEOF, p));
      p+=6;
      continue;
    }

    if(keyword(p, "do")){
      push_back(tokens, new_token_reserved(TK_DO, p));
      p+=2;
      continue;
    }
    
    // A variable
    if(valid_leading(*p)){
      int len;
      for(len =1; is_alnum(*(p+len)); len++){ }
      char *ident = strndup(p, len);
      push_back(tokens, new_token_ident(ident, p));
      p+=len;
      continue;
    }
    
    error("Line.%d in token.c : cannot tokenize %s", __LINE__, p);
  }
  
  push_back(tokens, new_token(TK_EOF, NULL, 0));
  return tokens;
}
