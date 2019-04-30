#include "arcc.h"

Vector *new_vector(){
  Vector *v = malloc(sizeof(Vector));
  v->data = malloc(sizeof(void*) * 16);
  v->cap = 16;
  v->len = 0;
  return v;
}

void add(Vector *v, void* elm){
  if(v->len == v->cap){
    v->cap *= 2;
    v->data = realloc(v->data ,sizeof(void*) * v->cap);
  }
  v->data[v->len++] = elm;
}
