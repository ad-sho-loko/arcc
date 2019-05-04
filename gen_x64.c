#include <stdio.h>
#include "arcc.h"

void gen_lval(Node *node){
  if(node->ty != TK_IDENT)
    error("左辺は変数でなければいけません");

  int offset = map_geti(map, node->name);
  out("mov rax, rbp");
  printf("  sub rax, %d\n", offset);
  out("push rax");
}

void gen(Node *node){
  if(node->ty == TK_IF){
    gen(node->cond);
    out("pop rax");
    out("cmp rax, 0");
    out("jne .Lelse0");
    gen(node->then);
    out(".Lelse0:");
    return;
  }

  if(node->ty == TK_BLOCK){
    for(int i=0; i<node->items->len; i++){
      gen(node->items->data[i]);
    }
    return;
  }
  
  if(node->ty == TK_NUM){
    printf("  push %d\n", node->val);
    return;
  }

  if(node->ty == TK_IDENT){
    gen_lval(node);
    out("pop rax");
    out("mov rax, [rax]");
    out("push rax");
    return;
  }

  if(node->ty == TK_RETURN){
    gen(node->lhs);
    out("pop rax");
    out("mov rsp, rbp");
    out("pop rbp");
    out("ret");
    return;
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
  case TK_EQL:
    out("cmp rax, rdi");
    out("sete al");
    out("movzb rax, al");
    break;
  case TK_NEQ:
    out("cmp rax, rdi");
    out("setne al");
    out("movzb rax, al");    
    break;
  case '<':
    out("cmp rax, rdi");
    out("setl al");
    out("movzb rax, al");    
    break;
  case TK_LE:
    out("cmp rax, rdi");
    out("setle al");
    out("movzb rax, al");
    break;
  }
  out("push rax");
}
