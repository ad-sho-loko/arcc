#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <stdarg.h>

enum{
  TK_NUM = 256,
  TK_EOF
};

typedef struct{
  int ty;
  int val;
  char *input;
} Token;

typedef struct Node{
  int ty;
  struct Node *lhs;
  struct Node *rhs;  
  int val;
} Node;

Token tokens[100];
Node *nodes[100];

void error(char* fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

Node *new_node(int ty, Node *lhs, Node *rhs){
  Node* n = malloc(sizeof(Node));
  n->ty = ty;
  n->lhs = lhs;
  n->rhs = rhs;
  return n;
}

Node *new_node_num(int val){
  Node* n = malloc(sizeof(Node));
  n->ty = TK_NUM;
  n->val = val;
  return n;
}

int pos = 0;

int consume(int ty){
  if(tokens[pos].ty != ty)
    return 0;
  pos++;
  return 1;
}

Node *add();
Node *mul();
Node *term();

Node* term(){
  if(consume('(')){
    Node *n = add();
    if(consume(')')){
      error("unexpected branket `)`");
      exit(1);
    }
    return n;
  }
  
  if(tokens[pos].ty == TK_NUM){
    return new_node_num(tokens[pos++].val);
  }

  error("数値でも開きカッコでもないトークンです: %s", tokens[pos].input);
  exit(1);
}

Node* mul(){
  Node *n = term();
  for(;;){
    if(consume('*')){
      n = new_node('*', n, term());
    }else if(consume('/')){
      n = new_node('/', n, term());
    }else{
      return n;
    }
  }
}

Node *add(){
  Node *n = mul();
  for(;;){
    if(consume('+')){
      n = new_node('+', n, mul());
    }else if(consume('-')){
      n = new_node('-', n, mul());
    }else{
      return n;
    } 
  }
}

void tokenize(char *p){
  int i = 0;
  while(*p){

    if(isspace(*p)){
      p++;
      continue;
    }
    
    if(*p == '+' || *p == '-' || *p == '*' || *p == '/' || *p == '(' || *p == ')'){
      tokens[i].ty = *p;
      tokens[i].input = p;
      i++;
      p++;
      continue;
    }

    if(isdigit(*p)){
      tokens[i].ty = TK_NUM;
      tokens[i].input = p;
      tokens[i].val = strtol(p, &p ,10);
      i++;
      continue;
    }

    error("cannot tokenize %c", p);
    exit(1);
  }

  tokens[i].ty = TK_NUM;
  tokens[i].input = p;
}

void gen(Node *node){
  if(node->ty == TK_NUM){
    printf("  push %d\n", node->val);
    return;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
  
  switch(node->ty){
  case '+':
    printf("  add rax, rdi\n");
    break;
  case '-':
    printf("  sub rax, rdi\n");
    break;
  case '*':
    printf("  mul rdi\n");
    break;
  case '/':
    printf("  mov rdx, 0\n");
    printf("  div rdi\n");
    break;
  }
  printf("  push rax\n");
}

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  printf(".intel_syntax noprefix\n");
  printf(".global _main\n");
  printf("_main:\n");

  tokenize(argv[1]);

  pos = 0;
  Node *n = add();
  gen(n);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;  
}
