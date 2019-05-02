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

  out("push rbp");
  out("mov rbp, rsp");
  out("sub rsp, 208");

  tokens = tokenize(argv[1]);
  program();

  for(int i=0; codes[i] != NULL; i++){
    gen(codes[i]);
    out("pop rax");
  }

  out("mov rsp, rbp");
  out("pop rbp");
  out("ret");
  return 0;  
}
