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

void outf(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  fprintf(stdout, "  ");
  vfprintf(stdout, fmt, ap);
  fprintf(stdout, "\n");
}

void outd(char *code){
  printf("## %s\n", code);
}

int do_align(int x, int align){
  return (x + align - 1) & ~(align - 1);
}

void printd(char *fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
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
    char *name;
    if(n->lhs == NULL){ lhs = "";} else { lhs = stringfy_node(n->lhs->ty);}
    if(n->rhs == NULL){ rhs = "";} else { rhs = stringfy_node(n->rhs->ty);}
    if(n->cond == NULL){ cond = "";} else { cond = stringfy_node(n->cond->ty);}
    if(n->name == NULL){name ="";}else {name = n->name;}
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
    fprintf(stderr, "nodes[%d]{ ty:%s, lhs:%s, rhs:%s name:%s, cond:%s, then:%s, els=%s}\n", i, stringfy_node(n->ty), lhs, rhs, name, cond, then, els);
  }
}

// map
Map* new_map(){
  Map* m = malloc(sizeof(Map));
  m->keys = new_vector();
  m->values = new_vector();
  return m;
}

void* map_get(Map *m, char *key){
  for(int i=0; i<m->keys->len; i++){
    if (strcmp(m->keys->data[i], key) == 0){
      return m->values->data[i];
    }
  }
  return NULL;
}

void map_put(Map *m, char *key, void *value) {
  if(map_get(m, key) != NULL){
    return;
  }

  push_back(m->keys, key);
  push_back(m->values, value);
}

void map_putv(Map *m, char *key, Var* value){
  map_put(m, key, value);
}

Var *map_getv(Map *m, char *key){
  return map_get(m, key);
}

void map_putm(Map *m, char *key, Map* value){
  map_put(m, key, value);
}

Map *map_getm(Map *m, char *key){
  return map_get(m, key);
}

int map_len(Map *m){
  return m->keys->len;
}

int map_contains(Map *m, char* key){
  if (map_len(m) == 0)
    return 0;
  return map_get(m, key) != NULL;
}

Stack *new_stack(){
  Stack *s = malloc(sizeof(Stack));
  s->items = new_vector();
  s->pos = 0;
  return s;
}

void stack_push(Stack *stack, void* v){
  stack->items->data[stack->pos] = v;
  stack->pos++;
}

void *stack_pop(Stack *stack){
  if(stack->pos <= 0) error("stack is no items.");
  return stack->items->data[--stack->pos];
}

void *stack_peek(Stack *stack){
  return stack->items->data[stack->pos-1];
}

static char* stringfy_ascii(char code){
  char *s = malloc(sizeof(char)*2);
  s[0] = code; s[1] = '\0';
  return s;
}

char* stringfy_token(int tkn_kind){
  if(tkn_kind <= 255) {
    return stringfy_ascii(tkn_kind);
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
  case 277: return "TK_BREAK";
  case 278: return "TK_CONTINUE";
  case 279: return "TK_INT";
  case 280: return "TK_PTR";
  case 281: return "TK_ADR";
  case 282: return "TK_TYPE";
  case 283: return "TK_REM_EQ";
  case 284: return "ND_LSHIFT";
  case 285: return "ND_RSHIFT";
  case 286: return "ND_LSHIFT_EQ";
  case 287: return "ND_RSHIFT_EQ";
  default: return "Unknown";
  }
}

char* stringfy_node(int node_kind){
  if(node_kind <= 255) {
    return stringfy_ascii(node_kind);
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
  case 276: return "ND_BREAK";
  case 277: return "ND_CONTINUE";
  case 278: return "ND_INT";
  case 279: return "ND_PTR";
  case 280: return "ND_ADR";
  case 281: return "ND_TYPE";
  case 282: return "ND_LSHIFT";
  case 283: return "ND_RSHIFT";
  default: return "Unknown";
  }
}

