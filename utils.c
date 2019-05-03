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

/*
void printd(char *s){
  fprintf(stderr, s);
}
*/

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
  for(int i=0; i<v->len; i++){
    Token* t = (Token*)v->data[i];
    fprintf(stderr, "token[%d]{ ty:%s, input:%s, val:%d, name:%s }\n", i,  stringfy_token(t->ty), t->input, t->val, t->name);
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
    return "CHAR"; 
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
  default: return "Unknown";
  }
}

