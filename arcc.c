#include <stdio.h>
#include "arcc.h"
Vector *nodes;
Vector *tokens;
Vector *strings;
Env *global_env;

void init(){
  nodes = new_vector();
  strings = new_vector();
  global_env = init_env();
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // initialize
  init();

  // tokenize
  tokens = tokenize(argv[1]);
  debug_vector_token(tokens);

  // parse
  toplevel();
  debug_vector_nodes(nodes);
  
  // generate x64
  gen_top();
  printd("===== A COMPILE FINISHED =====");
  return 0;  
}
