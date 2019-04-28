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

Token tokens[100];

void error(char* fmt, ...){
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void tokenize(char *p){
  int i = 0;
  while(*p){

    if(isspace(*p)){
      p++;
      continue;
    }
    
    if(*p == '+' || *p == '-'){
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


int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  printf(".intel_syntax noprefix\n");
  printf(".global _main\n");
  printf("_main:\n");

  tokenize(argv[1]);

  if(tokens[0].ty == TK_NUM){
    printf("  mov rax, %d\n", tokens[0].val);
  }

  int i=1;
  while(tokens[i].ty != TK_NUM){
    if(tokens[i].ty == '+'){
      i++;
      if(tokens[i].ty != TK_NUM) error("unexpected token %c\n", *tokens[i].input);
      printf("  add rax, %d\n", tokens[i].val);
      i++;
      continue;
    }

    if(tokens[i].ty == '-'){
      i++;
      if(tokens[i].ty != TK_NUM) error("unexpected token %c\n", *tokens[i].input);
      printf("  sub rax, %d\n", tokens[i].val);
      i++;
      continue;
    }
    
    fprintf(stderr, "unexpected token %c\n", *tokens[i].input);
  }
  
  printf("  ret\n");
  return 0;  
}
