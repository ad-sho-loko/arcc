#include <stdio.h>
#include <stdarg.h>
#include "arcc.h"

static int pos = 0;
static Map *local_env;

static void init_local_env(char *func_name){
  local_env = new_map();
  // todo : funcのかぶりvalidation
  map_putm(global_env, func_name, local_env);
}

static int get_type_sizeof(int type){
  switch(type){
  case INT: return 4;
  case PTR : return 8;
  }
  error("parse.c : Line.%d\n  ERROR :No such a type.");
  return 0;
}

static int get_node_sizeof(Node *n){
  if(n->ty == ND_NUM){
    return 4;
  }

  if(n->ty == ND_IDENT){
    Var *var = map_getv(local_env, n->name);
    if(var->type->ty == INT) return 4;
    if(var->type->ty == PTR) return 8;
  }

  error("parse.c : Line.%d\n ERROR : cannnot get sizeof %c", __LINE__, n->ty);
  return 0;
}

static Var *new_var(Type *type, char* name, int size){
  Var *v = malloc(sizeof(Var));
  v->type = type;
  v->pos = size;
  v->name = name;
  return v;
}

static Node *new_node(int ty, Node *lhs, Node *rhs){
  Node* n = malloc(sizeof(Node));
  n->ty = ty;
  n->lhs = lhs;
  n->rhs = rhs;
  return n;
}

static Node *new_node_empty(int ty){
  Node * n = malloc(sizeof(Node));
  n->ty = ty;
  return n;
}

static Node *new_node_ptr(int cnt, char* name){
  Node* n = malloc(sizeof(Node));
  n->ty = ND_PTR;
  n->cnt = cnt;
  n->name = name;
  return n;
}

static Node *new_node_adr(char* name){
  Node* n = malloc(sizeof(Node));
  n->ty = ND_ADR;
  n->name = name;
  return n;
}

static Node *new_node_num(int val){
  Node* n = malloc(sizeof(Node));
  n->ty = ND_NUM;
  n->val = val;
  return n;
}

static Node *new_node_dummy(){
  Node* n = malloc(sizeof(Node));
  n->ty = ND_DUMMY;
  return n;
}

static Node *new_node_init_ident(Type* type, char* name){
  // todo : 型のサイズによって変える
  map_putv(local_env, name, new_var(type, name, 8));
  // map_putv(local_env, name, new_var(type, name, get_type_sizeof(type->ty)));
  return new_node_dummy();
}

static Node *new_array_type(Type *type, char *name, int size){
  // todo : 型のサイズによって変える
  map_putv(local_env, name, new_var(type, name, 8 * size));
  return new_node_dummy();
}

static Node *new_node_ident(char* name){
  Node *n = malloc(sizeof(Node));
  n->ty = ND_IDENT;
  n->name = name;
  return n;
}

static Node *new_node_ident_array(char* name, int i){
  Node *n = malloc(sizeof(Node));
  n->ty = ND_IDENT;
  n->name = name;
  n->val = i;
  return n;
}

static Node *new_node_call_func(char* name){
  Node *n = malloc(sizeof(Node));
  n->ty = ND_FUNC;
  n->name = name;
  n->items = NULL;
  return n;
}

static Node *new_node_declare_func(char *name){
  Node *n = malloc(sizeof(Node));
  n->ty = ND_DEC_FUNC;
  n->arg_num = 0;
  n->name = name;
  return n;
}

static Token* tkn(){
  return ((Token*)(tokens->data[pos]));
}

static void next(){
  pos++;
}

void assume(int ty){
  int actual = ((Token*)(tokens->data[pos]))->ty;
  if(actual != ty){
    error("parse.c : Line.%d\n   expected=%c but accutal=%c %d  in parse.c", __LINE__, ty, actual, actual);
  }
}

int consume(int ty){
  if(((Token*)tokens->data[pos])->ty != ty)
    return 0;
  next();
  return 1;
}

void expect(int ty){
  int actual = tkn()->ty;
  if(actual != ty){
    error("parse.c : Line.%d\n  expected=%c but accutal=%c %d  in parse.c", __LINE__, ty, actual, actual);
  }
  next();
}

Token *expect2(int ty, char *fmt, ...){
  int actual = ((Token*)(tokens->data[pos]))->ty;
  if(actual != ty){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
  }
  Token *t = ((Token*)(tokens->data[pos]));
  next();
  return t;
}

Node* ident(Token *tkn){
  // WHEN calling function comes...
  if(consume('(')){
    Node *n = new_node_call_func(tkn->name);
    n->items = new_vector();
    while(((Token*)(tokens->data[pos]))->ty != ')'){
      push_back(n->items, equality());
      consume(',');
    }
    expect(')');
    return n;
  }
  
  // WHEN variable comes...
  if(!map_contains(local_env, tkn->name)){
    error("parse.c : Line %d \n  ERROR: name '%s' is not defined. ", __LINE__, tkn->name);
  }

  Node *n;
  // array
  if(consume('[')){
    int i = expect2(TK_NUM, "", __LINE__)->val;
    n = new_node_ident_array(tkn->name, i);
    expect(']');
  }else{
    // not array
    n = new_node_ident(tkn->name);
  }
  
  // post-inc, post-dec
  if(consume(TK_INC)){
    return new_node(ND_INC, n, NULL);
  }else if(consume(TK_DEC)){
    return new_node(ND_DEC, n, NULL);
  }
  return n;
}

Node* term(){
  if(consume('(')){
    Node *n = assign();
    if(!consume(')')){
      error("parse.c : Line %d \n  ERROR: unexpected branket `)`", __LINE__);
    }
    return n;
  }

  // The progress of implemention.
  // [o] int a;
  // [x] int a = 1;
  // [x] int a, b;
  // [x] int a = 1, b;
  // [x] int a = 1, b = 2;
  if(((Token*)(tokens->data[pos]))->ty == TK_TYPE){
    Type *type = ((Token*)(tokens->data[pos]))->type;
    expect(TK_TYPE);

    Token *t = ((Token*)(tokens->data[pos]));
    if(map_contains(local_env, t->name)){
      error("parse.c : Line %d \n  ERROR: name '%s' is already defined. ", __LINE__, t->name);
    }
    expect(TK_IDENT);

    /** Array **/
    if(consume('[')){
      Token *n_tk = expect2(TK_NUM, "parse.c : Line %d \n ERROR : A inner of array must be a number.", __LINE__);
      Node *n = new_array_type(type, t->name, n_tk->val);
      expect2(']', "parse.c : Line %d \n ERROR: An array must be closed.", __LINE__);
      return n;
    }

    return new_node_init_ident(type, t->name);
  }

  if(consume(TK_ADR)){
    Token *t = expect2(TK_IDENT, "parse.c : Line %d \n ERROR : アンパサンドのあとは必ず変数です", __LINE__);

    if(!map_contains(local_env, t->name)){
      error("parse.c : Line %d \n  ERROR: name '%s' is not defined. ", __LINE__, t->name);
    }
    return new_node_adr(t->name);
  }

  if(consume(TK_PTR)){
    int cnt = 1;
    while(consume(TK_PTR)){
      cnt++;
    }
    
    Token *t = expect2(TK_IDENT, "parse.c : Line %d \n ERROR : ポインタのあとは必ず変数です", __LINE__);
    if(!map_contains(local_env, t->name)){
      error("parse.c : Line %d \n  ERROR: name '%s' is not defined. ", __LINE__, t->name);
    }
    
    return new_node_ptr(cnt, t->name);
  }

  Token *tkn = ((Token*)(tokens->data[pos]));
  if(consume(TK_NUM)){
    return new_node_num(tkn->val);
  }
  
  if(consume(TK_IDENT)){
    return ident(tkn);
  }
  
  error("parse.c : Line.%d\n  ERROR: expected the token of 'number' or '(', but '%c %d'", __LINE__  , tkn->ty, tkn->ty);
  exit(1);
}

Node* unary(){
  if(consume(TK_INC)){
    assume(TK_IDENT);
    Node *ident = term();
    return new_node('=', ident, new_node('+', ident, new_node_num(1)));
  }else if(consume(TK_DEC)){
    assume(TK_IDENT);
    Node *ident = term();
    return new_node('=', ident, new_node('-', ident, new_node_num(1)));
  }else if(consume('+')){
    return term();
  }else if(consume('-')){
    return new_node('-', new_node_num(0), term());
  }else if(consume('~')){
    return new_node('~', term(), NULL);
  }else if(consume(TK_SIZEOF)){
    Node *n;
    int is_bracket = consume('(');
    if(tkn()->ty == TK_TYPE){
      Token *t = expect2(TK_TYPE, "parse.c : Line.%d\n ERROR : The program cannot reach here.", __LINE__);
      n = new_node_num(get_type_sizeof(t->type->ty));
    }else{
      n = unary();
    }
    if(is_bracket) expect2(')', "parse.c : Line.%d\n  ERROR : sizeofの括弧が閉じられていません ", __LINE__);
    return new_node_num(get_node_sizeof(n));
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
    }else if(consume('%')){
      n = new_node('%', n, unary());
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

Node* shift(){
  Node *n = add();
  for(;;){
    if(consume(TK_LSHIFT)){
      n = new_node(ND_LSHIFT, n, add());
    }else if(consume(TK_RSHIFT)){
      n = new_node(ND_RSHIFT, n, add());
    }else{
      return n;
    }
  }
}

Node* relational(){
  Node *n = shift();
  for(;;){
    if(consume('<')){
      n = new_node('<', n, shift());
    }else if(consume(TK_LE)){
      n = new_node(ND_LE, n, shift());
    }else if(consume('>')){
      n = new_node(ND_LE, shift(), n);
    }else if(consume(TK_GE)){
      n = new_node(ND_LE, shift(), n);
    }else{
      return n;      
    }
  }
}

Node *equality(){
  Node *n = relational();
  for(;;){
    if(consume(TK_EQL)){
      n = new_node(ND_EQL, n, relational());
    }else if(consume(TK_NEQ)){
      n = new_node(ND_NEQ, n, relational());
    }else{
      return n;
    }
  }
}

Node *bit(){
  Node *n = equality();
  for(;;){
    if(consume('&')){
      n = new_node('&', n, equality());
    }else if(consume('^')){
      n = new_node('^', n, equality());
    }else if(consume('|')){
      n = new_node('|', n, equality());
    }else{
      return n;
    }
  }
}

Node *expr(){
  Node *n = bit();
  for(;;){
    if(consume(TK_AND)){
      n = new_node(ND_AND, n, bit());
    }else if(consume(TK_OR)){
      n = new_node(ND_OR, n, bit());
    }else{
      return n;
    }
  }
}

Node *ternary(){
  Node *n = expr();
  if(consume('?')){
    Node* if_node = new_node(ND_IF, NULL, NULL);
    if_node->cond = n;
    if_node->then = ternary();
    expect(':');
    if_node->els = ternary();
    return if_node;
  }else{
    return n;
  }
}

Node *assign(){
  Node *n = ternary();
  for(;;){
    if(consume('=')){
      n = new_node('=', n, assign());
    }else if(consume(TK_PLUS_EQ)){
      n = new_node('=', n, new_node('+', n, assign()));
    }else if(consume(TK_MINUS_EQ)){
      n = new_node('=', n, new_node('-', n, assign()));
    }else if(consume(TK_MUL_EQ)){
      n = new_node('=', n, new_node('*', n, assign()));
    }else if(consume(TK_DIV_EQ)){
      n = new_node('=', n, new_node('/', n, assign()));
    }else if(consume(TK_REM_EQ)){
      n = new_node('=', n, new_node('%', n, assign()));
    }else if(consume(TK_LSHIFT_EQ)){
      n = new_node('=', n, new_node(ND_LSHIFT, n, assign()));
    }else if(consume(TK_RSHIFT_EQ)){
      n = new_node('=', n, new_node(ND_RSHIFT, n, assign()));
    }else if(consume(TK_AND_EQ)){
      n = new_node('=', n, new_node('&', n, assign()));
    }else if(consume(TK_OR_EQ)){
      n = new_node('=', n, new_node('|', n, assign()));
    }else if(consume(TK_XOR_EQ)){
      n = new_node('=', n, new_node('^', n, assign()));
    }else{
      return n;
    }
  }
}

Node *block(){
  Vector *v = new_vector();
  if(consume('{')){
    while(!consume('}')){
      Node *n = stmt();
      if(n->ty != ND_DUMMY) push_back(v, n);
    }
    Node *n = malloc(sizeof(Node));
    n->ty = ND_BLOCK;
    n->items = v;
    return n;
  }else{
    return stmt();
  }
}

Node *if_stmt(){
  expect('(');
  Node *n = new_node_empty(ND_IF);
  n->cond = expr();
  expect(')');
  n->then = block();
  if(consume(TK_ELSE_IF)){
    n->els = if_stmt();
    return n;
  }

  if(consume(TK_ELSE)){
    n->els = block();
  }
  return n;
}

Node *do_while_stmt(){
  Node *n = new_node_empty(ND_DO_WHILE);
  n->then = block();
  expect(TK_WHILE);
  expect('(');
  n->cond = expr();
  expect(')');
  expect(';');
  return n;
}

Node *while_stmt(){
  expect('(');
  Node *n = new_node(ND_WHILE, NULL, NULL);
  n->cond = expr();
  expect(')');
  n->then = block();
  return n;
}

Node *for_stmt(){
  expect('(');
  Node *n = new_node(ND_FOR, NULL, NULL);

  if(!consume(';')){
    n->init = assign();
    expect(';');
  }
  
  if(!consume(';')){
    n->cond = assign();
    expect(';');
  }
  
  if(!consume(')')){
    n->last = assign();
    expect(')');
  }
  
  n->then = block();
  return n;
}

Node *stmt(){
  Node* n;
  if(consume(TK_RETURN)){
    n = new_node(ND_RETURN, assign(), NULL);
    expect(';');
  }else if(consume(TK_IF)){
    n = if_stmt();
  }else if(consume(TK_WHILE)){
    n = while_stmt();
  }else if(consume(TK_FOR)){
    n = for_stmt();
  }else if(consume(TK_DO)){
    n = do_while_stmt();
  }else if(consume(TK_BREAK)){
    n = new_node_empty(ND_BREAK);
    expect(';');
  }else if(consume(TK_CONTINUE)){
    n = new_node_empty(ND_CONTINUE);
    expect(';');
  }else{
    n = assign();
    expect(';');
  }
  return n;
}

static void opt(Node *n){
  if(n->lhs != NULL) opt(n->lhs);
  if(n->rhs != NULL) opt(n->rhs);

  // ポインタの加減算を調整するためだけ....
  // TODO: 左にしかポインタおけない
  // TODO: 2つまでのデリファレンス（**p）しか対応していない?
  if(n->ty == '+' || n->ty == '-'){
    if(n->lhs != NULL && n->lhs->ty == ND_IDENT && n->rhs != NULL && n->rhs->ty == ND_NUM){
      Var *v = map_getv(local_env, n->lhs->name);
      if(v->type->ty == PTR)
        switch(v->type->ptr_of->ptr_of->ty){
        case INT:
          n->rhs->val = n->rhs->val * sizeof(int);
          break;
        case PTR:
          n->rhs->val = n->rhs->val * sizeof(int*);
          break;
        } else if(v->type->ty == INT){
          // nop
      }
    }
  }
}

void walk_nodes(){
  for(int i=0; i< nodes->len; i++){
    opt(nodes->data[i]);
  }
}

void func_body(){
  while(((Token*)tokens->data[pos])->ty != '}'){
    Node *n = stmt();
    if(n->ty != ND_DUMMY) push_back(nodes, n);
  }
}

void func(){
    // A return type
    expect2(TK_TYPE, "Line.%d in parse.c : 関数の宣言は型から始める必要があります", __LINE__);

    // A function name
    Token *t = expect2(TK_IDENT, "Line.%d in parse.c : 関数の宣言から始める必要があります", __LINE__);
    Node *n = new_node_declare_func(t->name);
    push_back(nodes, n);
    
    // Initialize (set a local environment)
    init_local_env(t->name);
    
    // Dummy Arguments
    expect('(');
    while(((Token*)tokens->data[pos])->ty != ')'){
      assume(TK_TYPE);
      term();
      consume(',');
      // todo : refine.
      n->arg_num++;
    }
    expect(')');

    // A function body
    expect('{');
    func_body();
    expect('}');    
    push_back(nodes, new_node(ND_FUNC_END, NULL, NULL));  
}
 
void toplevel(){
  while(tkn()->ty != TK_EOF){
    func();
    // NEXT : struct(union)
    // NEXT : global variables
    // NEXT : macro
  }

  // todo : Node構築後の作業 : ポインタの加減を調整する.
  walk_nodes();
}
