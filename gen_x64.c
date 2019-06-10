#include <stdio.h>
#include <string.h>
#include "arcc.h"

static int next_label = 1;
static Map* now_env;

typedef struct Env{
  int n;
  char *start;
  char *then;
  char *els;
  char *last;
  char *end;
} Env;

static char *new_label(char *sign, int cnt){
  char *s = malloc(sizeof(char)*256);
  snprintf(s, 256, ".L%s%03d", sign, cnt);
  return s;
}

static Env *new_env(int n){
  Env *e = malloc(sizeof(Env));
  e->n = n;
  e->start = new_label("begin", n);
  e->then = new_label("then", n);
  e->els = new_label("else", n);
  e->last = new_label("last", n);
  e->end = new_label("end", n);
  return e;
}

static int env_size(){
  int sum = 0;
  int len = now_env->values->len;
  for(int i=0; i<len; i++){
    sum += ((Var*)(now_env->values->data[i]))->pos;
  }
  return sum;
}

static Stack *env_stack;
static char *regs[2] = {"rdi", "rsi"};

// TODO : 出力にコメントをつける
void gen_top(){

  // init
  env_stack = new_stack();
  
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
      outf("sub rsp, %d", do_align(env_size(), 8));
      
      // todo : only two args.
      for(int i=0; i < n->arg_num; i++){
        outf("mov [rbp-%d], %s", (i+1)*8, regs[i]);
      }
    }else if(((Node*)nodes->data[i])->ty == ND_FUNC_END){
      // epiogue
      outd("epiogue");
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
  if(node->ty != ND_IDENT && node->ty != ND_PTR)
    error("Line.%d in gen_x64.c : 左辺は変数（ポインタ）でなければいけません", __LINE__);

  // ND_PTR
  // TODO : gen()のND_PTRのただのコピペ.
  if(node->ty == ND_PTR){
    int offset = map_getv(now_env, node->name)->pos * (map_indexOf(now_env, node->name) + 1);
    printf("  mov rax, [rbp-%d]\n", offset);
    for(int i=1; i < node->cnt; i++){
      out("mov rax, [rax]");
    }
    out("push rax");
    return;
  }

  // ND_IDENT
  int offset = map_getv(now_env, node->name)->pos * (map_indexOf(now_env, node->name) + 1);
  out("mov rax, rbp");
  outf("sub rax, %d", offset);
  out("push rax");
}

void gen(Node *node){  
  if(node->ty == ND_IF){
    gen(node->cond);
    out("pop rax");
    out("cmp rax, 1");

    int lcnt = next_label++;
    // todo : refactoring. 下を使う.
    // Env *e = new_env(lcnt);
    // stack_push(env_stack, e);
    
    if(node->els != NULL){
      outf("jne %s", new_label("else", lcnt));
    }else{
      outf("jne %s", new_label("end", lcnt));
    }

    // then
    gen(node->then);
    outf("jmp %s", new_label("end", lcnt));
    
    // e = stack_pop(env_stack);
    if(node->els != NULL){
      printf("%s:\n", new_label("else", lcnt));
      gen(node->els);
    }
    printf("%s:\n", new_label("end", lcnt));
    return;
  }

  if(node->ty == ND_WHILE){
    Env *e = new_env(next_label);
    next_label++;

    stack_push(env_stack, e);
    printf("%s:\n", e->start);
    printf("%s:\n", e->last); // forとの互換性のためwhileにも残しているが微妙.
    gen(node->cond);
    
    out("pop rax");
    out("cmp rax, 1");
    outf("jne %s", e->end);
    gen(node->then);
    e = stack_pop(env_stack);
    outf("jmp %s", e->start);
    printf("%s:\n", e->end);
    return;
  }

  if(node->ty == ND_DO_WHILE){
    Env *e = new_env(next_label);
    next_label++;
    stack_push(env_stack, e);
    printf("%s:\n", e->start);
    gen(node->then);
    gen(node->cond);
    out("pop rax");
    out("cmp rax, 1");
    outf("je %s", e->start);
    e = stack_pop(env_stack);
    return;
  }

  if(node->ty == ND_FOR){
    Env *e = new_env(next_label);
    next_label++;

    stack_push(env_stack, e);
    
    if(node->init != NULL){
      gen(node->init);
    }
    printf("%s:\n", e->start);
    
    if(node->cond != NULL){
      gen(node->cond);
      out("pop rax");
      out("cmp rax, 1");
      outf("jne %s", e->end);
    }
    
    gen(node->then);
    e = stack_pop(env_stack);
    
    printf("%s:\n", e->last);
    if(node->last != NULL){
      gen(node->last);
    }
    
    outf("jmp %s", e->start);
    printf("%s:\n", e->end);

    return; 
  }

  if(node->ty == ND_BREAK){
    outf("jmp %s", ((Env*)stack_peek(env_stack))->end);
    return;
  }

  if(node->ty == ND_CONTINUE){
    outf("jmp %s", ((Env*)stack_peek(env_stack))->last);
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
    // TODO : OFFSETの計算が行けてなさすぎる
    int offset = map_getv(now_env, node->name)->pos * (map_indexOf(now_env, node->name) + 1);
    printf("  lea rax, [rbp-%d]\n", offset);
    out("push rax");
    return;
  }

  // *x **x ***x
  if(node->ty == ND_PTR){
    int offset = map_getv(now_env, node->name)->pos * (map_indexOf(now_env, node->name) + 1);
    printf("  mov rax, [rbp-%d]\n", offset);
    for(int i=0; i < node->cnt; i++){
      out("mov rax, [rax]");
    }
    out("push rax");
    return;
  }

  if(node->ty == '~'){
    gen(node->lhs);
    out("pop rax");
    out("not rax");
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
        outf("pop %s", regs[i]);
      }
    }

    // TODO : align 16 byte.
    outf("call %s", node->name);
    out("push rax");
    return ;
  }

  if(node->ty == ND_OR){
    // todo : refactoring.
    gen(node->lhs);
    out("cmp rax, 1");
    char *_true = new_label("or_true", next_label);
    char *_false = new_label("or_false", next_label);
    char *end = new_label("or_end", next_label);
    next_label++;
    outf("je %s", _true);
    gen(node->rhs);
    out("cmp rax, 1");
    outf("jne %s", _false);
    printf("%s:\n", _true);
    out("push 1"); // true
    outf("jmp %s", end);
    printf("%s:\n", _false);
    out("push 0"); // false
    printf("%s:\n", end);
    return;
  }

  if(node->ty == ND_AND){
    // todo : refactoring.
    gen(node->lhs);
    out("cmp rax, 1");
    char *_false = new_label("and_false", next_label);
    char *end = new_label("and_end", next_label);
    next_label++;
    outf("jne %s", _false);
    gen(node->rhs);
    outf("jne %s", _false);
    out("push 1"); // true
    outf("jmp %s", end);
    printf("%s:\n", _false);
    out("push 0"); // false
    printf("%s:\n", end);
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
  case '&':
    // 3 & 1 
    out("and rax, rdi");
    break;
  case '|':
    // 3 | 1
    out("or rax, rdi");
    break;
  case '^':
    out("xor rax, rdi");
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
