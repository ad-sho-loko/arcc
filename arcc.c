#include <stdio.h>
#include "arcc.h"

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  printf(".intel_syntax noprefix\n");
  printf(".global _main\n");
  printf("_main:\n");

  tokenize(argv[1]);
  Node *n = equality();
  gen(n);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;  
}
