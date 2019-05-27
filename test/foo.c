#include <stdio.h>
#include <stdlib.h>

int foo(){
  return 5;
}

int *alloc4(){
  int *p = malloc(sizeof(int) * 4);
  p[0] = 1;
  p[1] = 2;
  p[2] = 4;
  p[3] = 8;
  return p;
}

int **alloc_ptr4(){
  int **p = malloc(sizeof(int*) * 4);
  int *i0 = malloc(sizeof(int));
  int *i1 = malloc(sizeof(int));
  int *i2 = malloc(sizeof(int));
  int *i3 = malloc(sizeof(int));
  *i0 = 1;
  *i1 = 2;
  *i2 = 4;
  *i3 = 8;
  p[0] = i0;
  p[1] = i1;
  p[2] = i2;
  p[3] = i3;
  return p;
}
