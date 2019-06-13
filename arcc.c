#include <stdio.h>
#include "arcc.h"
Vector *nodes;
Vector *tokens;
Map *global_env;

void init(){
  nodes = new_vector();
  global_env = new_map();
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
  debug_variable_table(nodes);

  // generate x64
  gen_top();
  printd("===== A COMPILE FINISHED =====");
  return 0;  
}
