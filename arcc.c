#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <strings.h>
#include <stdarg.h>

enum{
  TK_NUM = 256,
  TK_EQL,
  TK_NEQ,
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
Node *unary();
Node *equality();

Node* term(){
  if(consume('(')){
    Node *n = equality();
    if(!consume(')')){
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

Node* unary(){
  if(consume('+')){
    return term();
  }else if(consume('-')){
    return new_node('-', new_node_num(0), term());
  }
  return term();
}

Node* mul(){
  Node *n = unary();
  for(;;){
    if(consume('*')){
      n = new_node('*', n, unary());
    }else if(consume('/')){
      n = new_node('/', n, unary());
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

Node *equality(){
  Node *n = add();
  for(;;){
    if(consume(TK_EQL)){
      n = new_node(TK_EQL, n, add());
    }else if(consume(TK_NEQ)){
      n = new_node(TK_NEQ, n, add());
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

    if(*p == '=' && *(p+1) == '='){
      tokens[i].ty = TK_EQL;
      tokens[i].input = "==";
      i++;
      p+=2;
      continue;
    }

    if(*p == '!' && *(p+1) == '='){
      tokens[i].ty = TK_NEQ;
      tokens[i].input = "!=";
      i++;
      p+=2;
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
  case TK_EQL:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzx rax, al\n");
    break;
  case TK_NEQ:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzx rax, al\n");    
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
  Node *n = equality();
  gen(n);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;  
}
