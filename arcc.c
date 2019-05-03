#include <stdio.h>
#include "arcc.h"

Node *codes[100];
Vector *tokens;
Map *map;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  map = new_map();
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  out("push rbp");
  out("mov rbp, rsp");
  out("sub rsp, 208");
    
  tokens = tokenize(argv[1]);
  debug_vector_token(tokens);
  program();

  printf("  sub rsp, %d\n", (map_len(map) + 1) * 4);
  for(int i=0; codes[i] != NULL; i++){
    gen(codes[i]);
    out("pop rax");
  }

  out("mov rsp, rbp");
  out("pop rbp");
  out("ret");
  return 0;  
}
