#include <stdio.h>
#include "arcc.h"
Vector *nodes;
Vector *tokens;
Map *global_env;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  nodes = new_vector();
  global_env = new_map();
  
  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
    
  tokens = tokenize(argv[1]);
  debug_vector_token(tokens);
  toplevel();
  debug_vector_nodes(nodes);
  gen_top();

  return 0;  
}
