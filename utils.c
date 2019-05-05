#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include "arcc.h"

// common 
void error(char* fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void out(char* code){
  printf("  %s\n", code);
}

void printd(char *s){
  fprintf(stderr, s);
}

// vector
Vector *new_vector(){
  Vector *v = malloc(sizeof(Vector));
  v->data = malloc(sizeof(void*) * 16);
  v->cap = 16;
  v->len = 0;
  return v;
}

void push_back(Vector *v, void* elm){
  if(v->len == v->cap){
    v->cap *= 2;
    v->data = realloc(v->data ,sizeof(void*) * v->cap);
  }
  v->data[v->len++] = elm;
}

void push_backi(Vector *v, int elm){
  push_back(v, (void*)(intptr_t)elm);
}

void debug_vector_token(Vector *v){
  fprintf(stderr, "======Tokens======\n");
  for(int i=0; i<v->len; i++){
    Token* t = (Token*)v->data[i];
    fprintf(stderr, "token[%d]{ ty:%s, input:%s, val:%d, name:%s }\n", i,  stringfy_token(t->ty), t->input, t->val, t->name);
  }
}

void debug_vector_nodes(Vector *v){
  fprintf(stderr, "======Nodes======\n");
  for(int i=0; i<v->len; i++){
    Node *n = (Node*)v->data[i];
    char* lhs;
    char* rhs;
    char* cond;
    char* then;
    char* els;
    if(n->lhs == NULL){ lhs = "";} else { lhs = stringfy_node(n->lhs->ty);}
    if(n->rhs == NULL){ rhs = "";} else { rhs = stringfy_node(n->rhs->ty);}
    if(n->cond == NULL){ cond = "";} else { cond = stringfy_node(n->cond->ty);}
    if(n->then == NULL){ then = "";} else {
      then = stringfy_node(n->then->ty);
      if(n->then->ty == ND_BLOCK) {
        debug_vector_nodes(n->then->items);
      }
    }
    if(n->els == NULL){ els = "";} else{
      els = stringfy_node(n->els->ty);
      if(n->els->ty == ND_BLOCK){
        debug_vector_nodes(n->els->items);
      }
    }
    fprintf(stderr, "nodes[%d]{ ty:%s, lhs:%s, rhs:%s cond:%s, then:%s, els=%s}\n", i, stringfy_node(n->ty), lhs, rhs, cond, then, els);
  }
}

Map* new_map(){
  Map* m = malloc(sizeof(Map));
  m->keys = new_vector();
  m->values = new_vector();
  return m;
}

void map_put(Map *m, char *key, void *value) {
  push_back(m->keys, key);
  push_back(m->values, value);
}

void* map_get(Map *m, char *key){
  for(int i=0; i<m->keys->len; i++){
    if (strcmp(m->keys->data[i], key) == 0){
      return m->values->data[i];
    }
  }
  return NULL;
}

void map_puti(Map *m, char *key, int value){
  map_put(m, key, (void*)(intptr_t)value);
}

int map_geti(Map *m, char *key){
  return (intptr_t)map_get(m, key);
}

int map_len(Map *m){
  return m->keys->len;
}

char* stringfy_token(int tkn_kind){
  if(tkn_kind <= 255) {
    char *s = malloc(sizeof(char)*2);
    s[0] = (char)tkn_kind; s[1] = '\0';
    return s;
  }

  switch(tkn_kind){
  case 256: return "TK_NUM";
  case 257: return "TK_EQL";
  case 258: return "TK_NEQ";
  case 259: return "TK_LE";
  case 260: return "TK_GE";
  case 261: return "TK_IDENT";
  case 262: return "TK_RETERN";
  case 263: return "TK_EOF";
  case 264: return "TK_IF";
  case 265: return "TK_FOR";
  case 266: return "TK_WHILE";
  case 267: return "TK_ELSE";
  case 268: return "TK_AND";
  case 269: return "TK_OR";
  case 270: return "TK_INC";
  case 271: return "TK_DEC";
  case 272: return "TK_PLUS_EQ";
  case 273: return "TK_MINUS_EQ";
  case 274: return "TK_MUL_EQ";
  case 275: return "TK_DIV_EQ";
  case 276: return "TK_ELSE_IF";
  default: return "Unknown";
  }
}

char* stringfy_node(int node_kind){
  if(node_kind <= 255) {
    char *s = malloc(sizeof(char)*2);
    s[0] = (char)node_kind; s[1] = '\0';
    return s;
  }

  switch(node_kind){
  case 256: return "ND_NUM";
  case 257: return "ND_EQL";
  case 258: return "ND_NEQ";
  case 259: return "ND_LE";
  case 260: return "ND_GE";
  case 261: return "ND_IDENT";
  case 262: return "ND_RETERN";
  case 263: return "ND_EOF";
  case 264: return "ND_IF";
  case 265: return "ND_FOR";
  case 266: return "ND_WHILE";
  case 267: return "ND_ELSE";
  case 268: return "ND_BLOCK";
  case 269: return "ND_AND";    
  case 270: return "ND_OR";
  case 271: return "ND_INC";
  case 272: return "ND_DEC";
  case 273: return "ND_FUNC";
  case 274: return "ND_DEC_FUNC";
  case 275: return "ND_FUNC_END";
  default: return "Unknown";
  }
}

