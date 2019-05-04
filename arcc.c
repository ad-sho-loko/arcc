#include <stdio.h>
#include "arcc.h"
Vector *nodes;
Vector *tokens;
Map *map;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  nodes = new_vector();
  map = new_map();
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  out("push rbp");
  out("mov rbp, rsp");
    
  tokens = tokenize(argv[1]);
  debug_vector_token(tokens);
  program();
  debug_vector_nodes(nodes);
  printf("  sub rsp, %d\n", (map_len(map) + 1) * 4);
  
  for(int i=0; i<nodes->len; i++){
    gen(nodes->data[i]);
    out("pop rax");
  }

  out("mov rsp, rbp");
  out("pop rbp");
  out("ret");
  return 0;  
}
