#include <stdio.h>
#include <string.h>
#include "arcc.h"

typedef struct {
  Map *table;
  int size;
} VarTable;

typedef struct {
  Type *type;
  int size;
  int offset;
} VarDesc;

static int next_label = 1;
static VarTable* var_table;


static int get_type_sizeof(Type *type){
  if(type->ty == INT){
    return 4;
  }

  if(type->ty == PTR){
    return 8;
  }

  if(type->ty == ARRAY){
    return type->array_size * get_type_sizeof(type->ptr_of);
  }
  
  error("parse.c : Line.%d\n  ERROR :No such a type.");
  return 0;
}

static VarDesc *new_var_desc(Type *type, int size, int offset){
  VarDesc *v = malloc(sizeof(VarDesc));
  v->type = type;
  v->size = size;
  v->offset = offset;
  return v;
}

static VarDesc *lookup(char *name){
  return map_get(var_table->table, name);
}

static VarTable* create_var_table(Map *env){
  Map* m = new_map();
  int offset = 0;
  for(int i=0; i < env->values->len; i++){
    Var *v = ((Var*)env->values->data[i]);
    offset+= get_type_sizeof(v->type);
    map_put(m, v->name, new_var_desc(v->type, get_type_sizeof(v->type),  offset));
  }

  VarTable *tbl = malloc(sizeof(VarTable));
  tbl->table = m;
  tbl->size = offset;
  return tbl;
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

// NOT COOL....
void adjust(char *reg, Type* type){
  char s[64];
  if(type->ty == PTR){
    if(type->ptr_of->ty == INT){
      sprintf(s, "add %s, %s", reg, reg);
      out(s);
      out(s);
    }else{
      sprintf(s, "add %s, %s", reg, reg);
      out(s);
      out(s);
      out(s);
    }
  }
}

static Stack *env_stack;

// todo : now only unitl 2 args
static char *regs[2] = {"rdi", "rsi"};

// todo : refactoring
static char *reg[2][9] = {
  /* 1st */  {"","","","","edi","","","","rdi",},
  /* 2nd */  {"","","","","esi","","","","rsi",}
};

static char *mod[9] = {"","BYTE PTR","","","DWORD PTR","","","","QWORD PTR"};
static char *from[9] = {"","","","","edi","","","","rdi"};

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

      // The scope changed.
      var_table = create_var_table(map_getm(global_env, n->name));
      
      printf("%s:\n",n->name);
      out("push rbp");
      out("mov rbp, rsp");
      outf("sub rsp, %d", do_align(var_table->size, 16));

      // Move args from register into stack.
      for(int i=0; i < n->arg_num; i++){
        char *key = var_table->table->keys->data[i];
        int size = lookup(key)->size;
        outf("mov %s [rbp-%d], %s", mod[size] , (i+1) * size, reg[i][size]);
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

// 左辺の評価
int gen_lval(Node *node){
  if(node->ty != ND_IDENT && node->ty != ND_DEREF){
    error("ERROR : LVAL ERROR = %d", node->ty);
  }

  if(node->ty == ND_DEREF){
    // 左辺のデリファレンス値は右辺値として評価する必要がある
    gen(node->lhs);
    return 8;
  }

  // ND_IDENT
  int offset = lookup(node->name)->offset;
  out("mov rax, rbp");
  outf("sub rax, %d", offset); 
  out("push rax");
  return lookup(node->name)->size;
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
    int size = gen_lval(node->lhs);
    gen(node->rhs);
    out("pop rdi");
    out("pop rax");
    outf("mov %s [rax], %s", mod[size], from[size]);
    out("push rdi");
    return;
  }

  // &x
  if(node->ty == ND_ADR){
    gen_lval(node->lhs);
    out("pop rax");
    outf("lea rax, [rax]");
    out("push rax");
    return;
  }

  // *x
  if(node->ty == ND_DEREF){
    gen(node->lhs);
    out("pop rax");
    out("mov rax, [rax]");
    out("push rax");
    return;
  }

  // x
  if(node->ty == ND_IDENT){
    // 左辺値として評価したのち、右辺値に変換している
    int size = gen_lval(node);

    /* Transform array into pointer after the array size was allocated. */
    if(lookup(node->name)->type->ty == ARRAY){
      return;
    }
    
    // [HERE] A variable address at the top of stack.
    out("pop rax");

    /*** TODO: Refactring ***/
    if(size < 8){
      outf("movsx rax, %s [rax]", mod[size]);
    }else{
      out("mov rax, [rax]");
    } 
    out("push rax");

    // [HERE] A actual variable value at the top of stack.
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
    char *ok = new_label("or_true", next_label);
    char *ng = new_label("or_false", next_label);
    char *end = new_label("or_end", next_label);
    next_label++;
    outf("je %s", ok);
    gen(node->rhs);
    out("cmp rax, 1");
    outf("jne %s", ng);
    printf("%s:\n", ok);
    out("push 1"); // true
    outf("jmp %s", end);
    printf("%s:\n", ng);
    out("push 0"); // false
    printf("%s:\n", end);
    return;
  }

  if(node->ty == ND_AND){
    // todo : refactoring.
    gen(node->lhs);
    out("cmp rax, 1");
    char *ng = new_label("and_false", next_label);
    char *end = new_label("and_end", next_label);
    next_label++;
    outf("jne %s", ng);
    gen(node->rhs);
    outf("jne %s", ng);
    out("push 1"); // true
    outf("jmp %s", end);
    printf("%s:\n", ng);
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
    if(node->lhs->ty == ND_IDENT && lookup(node->lhs->name)->type->ty == PTR){
      adjust("rdi", lookup(node->lhs->name)->type);
    }
    out("add rax, rdi");
    break;
  case '-':
    if(node->lhs->ty == ND_IDENT && lookup(node->lhs->name)->type->ty == PTR){
      adjust("rdi", lookup(node->lhs->name)->type);
    }
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
