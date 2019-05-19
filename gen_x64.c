#include <stdio.h>
#include <string.h>
#include "arcc.h"

static int next_label = 1;
static Map* now_env;
static char* now_scope_start;
static char* now_scope_last;
static char* now_scope_end;
static char *regs[2] = {"rdi", "rsi"};


static char *new_label(char *sign, int cnt){
  char *s = malloc(sizeof(char)*256);
  snprintf(s, 256, ".L%s%03d", sign, cnt);
  return s;
}

void gen_top(){

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  
  for(int i=0; i<nodes->len; i++){
    if(((Node*)nodes->data[i])->ty == ND_DEC_FUNC){
      // prologue
      Node *n = (Node*)nodes->data[i];
      now_env = map_getm(global_env, n->name);

      printf("%s:\n",n->name);
      out("push rbp");
      out("mov rbp, rsp");
      printf("  sub rsp, %d\n", do_align((map_len(now_env)) * 8, 8));

      // todo : only two args.
      for(int i=0; i < n->arg_num; i++){
        printf("  mov [rbp-%d], %s\n", (i+1)*8, regs[i]);
      }
    }else if(((Node*)nodes->data[i])->ty == ND_FUNC_END){
      // epiogue
      out("mov rsp, rbp");
      out("pop rbp");
      out("ret");
    }else{
      // body
      gen(nodes->data[i]);
      out("pop rax");
    }
  }
}

void gen_lval(Node *node){
  if(node->ty != ND_IDENT)
    error("Line.%d in gen.c : 左辺は変数でなければいけません", __LINE__);

  int offset = map_geti(now_env, node->name);
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
    out("push rax");
    return;
  }

  if(node->ty == ND_WHILE){
    int lcnt = next_label++;
    now_scope_start = new_label("begin", lcnt);
    now_scope_last = new_label("last", lcnt);
    now_scope_end = new_label("end", lcnt);

    printf("%s:\n", now_scope_start);
    printf("%s:\n", now_scope_last); // forとの互換性のためwhileにも残しているが微妙.
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
    now_scope_start = new_label("begin", lcnt);
    now_scope_last = new_label("last", lcnt);
    now_scope_end = new_label("end", lcnt);
    if(node->init != NULL){
      gen(node->init);
    }
    printf("%s:\n", now_scope_start);
    
    if(node->cond != NULL){
      gen(node->cond);
      out("pop rax");
      out("cmp rax, 1");
      printf("  jne %s\n", new_label("end", lcnt));
    }

    gen(node->then);
    printf("%s:\n", now_scope_last);
    if(node->last != NULL){
      gen(node->last);
    }
    
    printf("  jmp %s\n", now_scope_start);
    printf("%s:\n", now_scope_end);
    return; 
  }

  if(node->ty == ND_BREAK){
    printf("  jmp %s\n", now_scope_end);
    return;
  }

  if(node->ty == ND_CONTINUE){
    printf("  jmp %s\n", now_scope_last);
    return;
  }
  
  if(node->ty == ND_BLOCK){
    for(int i=0; i < node->items->len; i++){
      gen(node->items->data[i]);
    }
    return;
  }
  
  if(node->ty == ND_NUM){
    printf("  push %d\n", node->val);
    return;
  }

  // &x
  if(node->ty == ND_ADR){
    int offset = map_geti(now_env, node->name);
    out("mov rax, rbp");
    printf("  sub rax, %d\n", offset);
    out("push rax");
    return;
  }

  // *x
  if(node->ty == ND_PTR){
    int offset = map_geti(now_env, node->name);
    out("mov rax, rbp");
    printf("  sub rax, %d\n", offset);
    out("mov rax, [rax]");
    out("mov rax, [rax]");
    out("push rax");
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

  if(node->ty == ND_INC){
    gen_lval(node->lhs);
    out("pop rax");
    out("push [rax]");
    out("mov rdi, rax");
    out("mov rax, [rax]");
    out("add rax, 1");
    out("mov [rdi], rax");
    return;
  }

  if(node->ty == ND_DEC){
    gen_lval(node->lhs);
    out("pop rax");
    out("push [rax]");
    out("mov rdi, rax");
    out("mov rax, [rax]");
    out("sub rax, 1");
    out("mov [rdi], rax");
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
    // if args exists
    if(node->items != NULL){
      for(int i=0; i<node->items->len; i++){
        // todo : now only unitl 2 args
        gen(node->items->data[i]);
        printf("  pop %s\n", regs[i]);
      }
    }

    // TODO : align 16 byte.
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
  case '%':
    out("mov rdx, 0");
    out("div rdi");
    out("mov rax, rdx");
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
    // 1 && 1
    out("and rax, rdi");
    break;
  case ND_OR:
    // 1 || 1
    out("or rax, rdi");
    break;
  case ND_LSHIFT:
    out("mov rcx, rdi");    
    out("sal rax, cl");
    break;
  case ND_RSHIFT:
    out("mov rcx, rdi");    
    out("sar rax, cl");
    break;
  }
  out("push rax");
}
