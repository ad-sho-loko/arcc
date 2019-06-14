#include <stdio.h>
#include <string.h>
#include "arcc.h"


static int next_label = 1;
static Map* now_env;
static Map* var_table;

Map* new_var_table(Map *env){
  Map* m = new_map();
  int offset = 0;
  for(int i=0; i<env->values->len; i++){
    Var *v = ((Var*)env->values->data[i]);
    offset+=v->alloc_size;
    map_puti(m, v->name, offset);
  }
  return m;
}

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
    sum += ((Var*)(now_env->values->data[i]))->alloc_size;
  }
  return sum;
}

static Stack *env_stack;


// todo : refactoring.
static char *regs[2] = {"rdi", "rsi"};

static char *reg[2][9] = {
  /* 1st  */ {"","","","","edi","","","","rdi",},
  /* 2nd */  {"","","","","esi","","","","rsi",}
};

static char *bit[9] = {"","BYTE PTR","","","DWORD PTR","","","","QWORD PTR"};
static char *sreg[9] = {"","","","","edi","","","","rdi"};

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
      var_table = new_var_table(now_env);

      printf("%s:\n",n->name);
      out("push rbp");
      out("mov rbp, rsp");
      outf("sub rsp, %d", do_align(env_size(), 16));

      // args
      for(int i=0; i < n->arg_num; i++){
        char *key = now_env->keys->data[i];
        Var* v = map_getv(now_env, key);
        outf("mov %s [rbp-%d], %s", bit[v->alloc_size] , (i+1) * v->alloc_size, reg[i][v->alloc_size]);
      }
    }else if(((Node*)nodes->data[i])->ty == ND_FUNC_END){
      // epiogue
      outd("epiogue");
      out("leave");
      out("ret");
    }else{
      // body
      gen(nodes->data[i]);
      out("pop rax");
    }
  }
}

void gen_lval(Node *node){
  if(node->ty != ND_IDENT && node->ty != ND_ADR && node->ty != ND_DEREF)
    error("Line.%d in gen_x64.c : 左辺は変数でなければいけません", __LINE__);

  int offset = map_geti(var_table, node->name);
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

    Var *v = map_getv(now_env, node->lhs->name);
    int size;

    /** TODO : refactoring. **/
    if(v->type->ty == ARRAY){
      size = 4;
    }else{
      size = v->alloc_size;
    }
    outf("mov %s [rax], %s", bit[size], sreg[size]);
    out("push rdi");
    return;
  }

  // &x
  if(node->ty == ND_ADR){
    gen_lval(node);
    out("pop rax");
    outf("lea rax, [rax]");
    out("push rax");
    return;
  }

  // *x
  if(node->ty == ND_DEREF){
    gen_lval(node);
    out("pop rax");
    out("mov rax, [rax]");
    for(int i=0; i < node->cnt; i++){
      out("mov rax, [rax]");
    }
    out("push rax");
    return;
  }

  // x
  if(node->ty == ND_IDENT){
    gen_lval(node);
    // [HERE] A variable address in the top of stack.

    out("pop rax");
    int size = map_getv(now_env, node->name)->alloc_size;

    /** TODO : refactoring. **/
    if(size < 8){
      outf("movsx rax, %s [rax]",  bit[size]);
    }else{
      out("mov rax, [rax]");
    }
    
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
    out("leave");
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
    outf("call %s", node->name);
    out("push rax");
    return;
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
