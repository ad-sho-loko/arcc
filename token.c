#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include "arcc.h"

static bool is_var_leading(char ch){
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

static Token *new_token_op(int ty, char* input){
  Token *t = malloc(sizeof(Token));
  t->ty = ty;
  t->input = input;
  t->val = 0;
  return t;
}

static Token *new_token_reserved(int ty, char* input){
  Token *t = malloc(sizeof(Token));
  t->ty = ty;
  t->input = input;
  t->val = 0;
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

static Token *new_token_type(int ty, int type, char* input){
  Token *t = malloc(sizeof(Token));
  t->ty = ty;
  t->input = input;
  t->val = 0;
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

static Type* new_ptr(){
  Type *t = malloc(sizeof(Type));
  t->ty = TK_PTR;
  return t;
}

static Vector *pointernized(Vector* tokens){
  Vector* v = new_vector();
  for(int i=0; i<tokens->len; i++){
    if( ((Token*)(tokens->data[i]))->ty == TK_TYPE && ( i+1 < tokens->len && ((Token*)(tokens->data[i+1]))->ty == TK_PTR) ){

      // todo : refatoring
      // 複数ポインタへの対応
      Token *top = ((Token*)(tokens->data[i]));
      Type *last_type = top->type;
      top->type = new_ptr();
      Type *now = top->type;
      i++;
      while(((Token*)(tokens->data[i]))->ty == TK_PTR){
        now->ptr_of = new_ptr();
        now = now->ptr_of;
        i++;
      }
      now->ptr_of = last_type;
      push_back(v, top);
      i--;
      
    }else{
      push_back(v, tokens->data[i]);
    }
  }
  return v;
}

Vector *tokenize(char *p){
  Vector* tokens = new_vector();

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
    
    // pointer
    if(*p == '*' && (is_var_leading(*(p+1)) || *(p+1) == '*')){
      push_back(tokens, new_token(TK_PTR, "*", 0));
      p+=1;
      continue;
    }

    // reference
    if(*p == '&' && is_var_leading(*(p+1))){
      push_back(tokens, new_token(TK_ADR, "&", 0));
      p+=1;
      continue;
    }
    
    
    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')' || *p == '<' || *p == '>' || *p == ';' || *p == '=' || *p == '{' || *p == '}' || *p == ',' || *p == '%' || *p == '&' || *p == '|'){
      push_back(tokens, new_token(*p, p, 0));
      p++;
      continue;
    }

    if(isdigit(*p)){
      push_back(tokens, new_token(TK_NUM, p, strtol(p, &p ,10)));
      continue;
    }

    if(strncmp(p, "return", 6) == 0 && !is_alnum(p[6])){
      push_back(tokens, new_token_reserved(TK_RETURN, p));
      p+=6;
      continue;
    }

    if(strncmp(p, "if", 2) == 0 && !is_alnum(p[2])){
      push_back(tokens, new_token_reserved(TK_IF, p));
      p+=2;
      continue;
    }

    if(strncmp(p, "else if", 7) == 0 && !is_alnum(p[7])){
      push_back(tokens, new_token_reserved(TK_ELSE_IF, p));
      p+=7;
      continue;
    }
    
    if(strncmp(p, "else", 4) == 0 && !is_alnum(p[4])){
      push_back(tokens, new_token_reserved(TK_ELSE, p));
      p+=4;
      continue;
    }

    if(strncmp(p, "while", 5) == 0 && !is_alnum(p[5])){
      push_back(tokens, new_token_reserved(TK_WHILE, p));
      p+=5;
      continue;
    }    

    if(strncmp(p, "for", 3) == 0 && !is_alnum(p[3])){
      push_back(tokens, new_token_reserved(TK_FOR, p));
      p+=3;
      continue;
    }    

    if(strncmp(p, "break", 5) == 0 && !is_alnum(p[5])){
      push_back(tokens, new_token_reserved(TK_BREAK, p));
      p+=5;
      continue;
    }    

    if(strncmp(p, "continue", 8) == 0 && !is_alnum(p[8])){
      push_back(tokens, new_token_reserved(TK_CONTINUE, p));
      p+=8;
      continue;
    }    

    if(strncmp(p, "int", 3) == 0 && !is_alnum(p[3])){
      push_back(tokens, new_token_type(TK_TYPE, TK_INT, p));
      p+=3;
      continue;
    }    
    
    if(is_var_leading(*p)){
      int len = 1;
      while(is_alnum(*(p+len))){
        len++;
      }
      char *ident = strndup(p, len);
      push_back(tokens, new_token_ident(ident, p));
      p+=len;
      continue;
    }
    
    error("Line.%d in token.c : cannot tokenize %s", __LINE__, p);
  }
  push_back(tokens, new_token(TK_EOF, p, 0));
  return pointernized(tokens);
}
