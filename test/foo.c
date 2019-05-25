#include <stdio.h>
#include <stdlib.h>

int foo(){
  return 5;
}

int* alloc4(){
  int *p = malloc(sizeof(int) * 4);
  p[0] = 1;
  p[1] = 2;
  p[2] = 4;
  p[3] = 8;
  return p;
}
