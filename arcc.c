#include <stdio.h>
#include "arcc.h"

Node *codes[100];
Vector *tokens;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");
  
  tokens = tokenize(argv[1]);  
  program();

  for(int i=0; codes[i] != NULL; i++){
    gen(codes[i]);
  }

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;  
}
