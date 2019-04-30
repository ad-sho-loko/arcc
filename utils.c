#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "arcc.h"

// common 
void error(char* fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
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

