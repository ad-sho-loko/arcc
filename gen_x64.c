#include <stdio.h>
#include <string.h>
#include "arcc.h"

static int next_label = 1;

static char *new_label(char *sign, int cnt){
  char *s = malloc(sizeof(char)*256);
  snprintf(s, 256, ".L%s%03d", sign, cnt);
  return s;
}

void gen_lval(Node *node){
  if(node->ty != ND_IDENT)
    error("左辺は変数でなければいけません");

  int offset = map_geti(map, node->name);
  out("mov rax, rbp");
  printf("  sub rax, %d\n", offset);
  out("push rax");
}

void gen(Node *node){
  if(node->ty == ND_IF){
    gen(node->cond);
    out("pop rax");
    out("cmp rax, 1");

    int lcnt = next_label++;

    if(node->els != NULL){
      printf("  jne %s\n", new_label("else", lcnt));
    }else{
      printf("  jne %s\n", new_label("end", lcnt));
    }
    
    gen(node->then);

    if(node->els != NULL){
      printf("%s:\n", new_label("else", lcnt));
      gen(node->els);
      printf("  jmp %s\n", new_label("end", lcnt));      
    }
    printf("%s:\n", new_label("end", lcnt));
    return;
  }

  if(node->ty == ND_WHILE){
    int lcnt = next_label++;
    printf("%s:\n", new_label("begin", lcnt));
    gen(node->cond);
    out("pop rax");
    out("cmp rax, 1");
    printf("  jne %s\n", new_label("end", lcnt));
    gen(node->then);
    printf("  jmp %s\n", new_label("begin", lcnt));
    printf("%s:\n", new_label("end", lcnt));
    return;
  }

  if(node->ty == ND_FOR){
    int lcnt = next_label++;
    if(node->init != NULL){
      gen(node->init);
    }
    
    printf("%s:\n", new_label("begin", lcnt));

    if(node->cond != NULL){
      gen(node->cond);
      out("pop rax");
      out("cmp rax, 1");
      printf("  jne %s\n", new_label("end", lcnt));
    }
    
    gen(node->then);
    if(node->last != NULL){
      gen(node->last);
    }
    
    printf("  jmp %s\n", new_label("begin", lcnt));
    printf("%s:\n", new_label("end", lcnt));
    return; 
  }
  
  if(node->ty == ND_BLOCK){
    for(int i=0; i<node->items->len; i++){
      gen(node->items->data[i]);
    }
    return;
  }
  
  if(node->ty == ND_NUM){
    printf("  push %d\n", node->val);
    return;
  }

  if(node->ty == '='){
    gen_lval(node->lhs);
    gen(node->rhs);
    out("pop rdi");
    out("pop rax");
    out("mov [rax], rdi");
    out("push rdi");
    return;
  }
  
  if(node->ty == ND_IDENT){
    gen_lval(node);
    out("pop rax");
    out("mov rax, [rax]");
    out("push rax");
    return;
  }

  if(node->ty == ND_RETURN){
    gen(node->lhs);
    out("pop rax");
    out("mov rsp, rbp");
    out("pop rbp");
    out("ret");
    return;
  }

  if(node->ty == ND_FUNC){
    printf("  call %s\n", node->name);
    out("push rax");
    return ;
  }
  
  gen(node->lhs);
  gen(node->rhs);

  out("pop rdi");
  out("pop rax");
  
  switch(node->ty){
  case '+':
    out("add rax, rdi");
    break;
  case '-':
    out("sub rax, rdi");
    break;
  case '*':
    out("mul rdi");
    break;
  case '/':
    out("mov rdx, 0");
    out("div rdi");
    break;
  case ND_EQL:
    out("cmp rax, rdi");
    out("sete al");
    out("movzb rax, al");
    break;
  case ND_NEQ:
    out("cmp rax, rdi");
    out("setne al");
    out("movzb rax, al");    
    break;
  case '<':
    out("cmp rax, rdi");
    out("setl al");
    out("movzb rax, al");    
    break;
  case ND_LE:
    out("cmp rax, rdi");
    out("setle al");
    out("movzb rax, al");
    break;
  case ND_AND:
    out("and rax, rdi");
    break;
  case ND_OR:
    out("or rax, rdi");
    break;
  }
  out("push rax");
}
