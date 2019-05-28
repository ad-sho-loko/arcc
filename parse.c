#include <stdio.h>
#include <stdarg.h>
#include "arcc.h"

static int pos = 0;
static Map *local_env;

Var *new_var(Type *type, char* name, int pos){
  Var *v = malloc(sizeof(Var));
  v->type = type;
  v->pos = pos;
  v->name = name;
  return v;
}

Node *new_node(int ty, Node *lhs, Node *rhs){
  Node* n = malloc(sizeof(Node));
  n->ty = ty;
  n->lhs = lhs;
  n->rhs = rhs;
  return n;
}

Node *new_node_ptr(int cnt){
  Node* n = malloc(sizeof(Node));
  n->ty = ND_PTR;
  n->cnt = cnt;
  return n;
}

Node *new_node_num(int val){
  Node* n = malloc(sizeof(Node));
  n->ty = ND_NUM;
  n->val = val;
  return n;
}

Node *new_node_init_ident(Type* type, char* name){
  Node *n = malloc(sizeof(Node));
  n->ty = ND_IDENT;
  n->name = name;
  map_putv(local_env, name, new_var(type, name, (map_len(local_env) + 1) * 8));
  return n;
}

Node *new_node_ident(char* name){
  Node *n = malloc(sizeof(Node));
  n->ty = ND_IDENT;
  n->name = name;
  return n;
}

Node *new_node_func(char* name){
  Node *n = malloc(sizeof(Node));
  n->ty = ND_FUNC;
  n->name = name;
  n->items = NULL;
  return n;
}

Node *new_node_declare_func(char *name){
  Node *n = malloc(sizeof(Node));
  n->ty = ND_DEC_FUNC;
  n->arg_num = 0;
  n->name = name;
  return n;
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
  int actual = ((Token*)(tokens->data[pos]))->ty;
  if(actual != ty){
    error("parse.c : Line.%d\n  expected=%c but accutal=%c %d  in parse.c", __LINE__, ty, actual, actual);
  }
  next();
}

void expect2(int ty, char *fmt, ...){
  int actual = ((Token*)(tokens->data[pos]))->ty;
  if(actual != ty){
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
  }
  next();
}

static int get_sizeof(Node *n){
  if(n->ty == ND_NUM){
    return 4;
  }

  if(n->ty == ND_IDENT){
    Var *var = map_getv(local_env, n->name);
    if(var->type->ty == INT) return 4;
    if(var->type->ty == PTR) return 8;
  }

  error("parse.c : Line.%d\n  cannnot get sizeof %c", __LINE__, n->ty);
  return 0;
}

Node* ident(Token *tkn){
  // WHEN calling function comes...
  if(consume('(')){
    Node *n = new_node_func(tkn->name);
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
  
  Node *n = new_node_ident(tkn->name);
  // 優先度TOPの演算子の処理
  if(consume(TK_INC)){
    // a++ -> a; a+=1;
    return new_node(ND_INC, n, NULL);
  }else if(consume(TK_DEC)){
    // a-- -> a; a-=1
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

    /*
    if(((Token*)(tokens->data[pos]))->ty != TK_IDENT){
      error("parse.c : Line %d \n  型宣言のあとは変数でなければいけません", __LINE__);
    }
    */

    // 同じ変数名の宣言はエラー
    Token *t = ((Token*)(tokens->data[pos]));
    if(map_contains(local_env, t->name)){
      error("parse.c : Line %d \n  ERROR: name '%s' is already defined. ", __LINE__, t->name);
    }

    Node *n = new_node_init_ident(type, t->name);
    expect(TK_IDENT);
    return n;
  }

  if(consume(TK_ADR)){
    Token *t = ((Token*)(tokens->data[pos]));
    expect2(TK_IDENT, "parse.c : Line %d \n ERROR : アンパサンドのあとは必ず変数です", __LINE__);

    if(!map_contains(local_env, t->name)){
      error("parse.c : Line %d \n  ERROR: name '%s' is not defined. ", __LINE__, t->name);
    }
    Node *n = new_node(ND_ADR, NULL, NULL);
    n->name = t->name;
    return n;
  }

  if(consume(TK_PTR)){
    int cnt = 1;
    while(consume(TK_PTR)){
      cnt++;
    }
    
    Token *t = ((Token*)(tokens->data[pos]));
    expect2(TK_IDENT, "parse.c : Line %d \n ERROR : ポインタのあとは必ず変数です", __LINE__);

    if(!map_contains(local_env, t->name)){
      error("parse.c : Line %d \n  ERROR: name '%s' is not defined. ", __LINE__, t->name);
    }
    
    Node *n = new_node_ptr(cnt);
    n->name = t->name;
    return n;
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
    // [HERE] ++a -> a = a + 1;
    // [NOT] a++;
    assume(TK_IDENT);
    Node *ident = term();
    return new_node('=', ident, new_node('+', ident, new_node_num(1)));
  }else if(consume(TK_DEC)){
    // [HERE] --a -> a = a - 1;
    assume(TK_IDENT);
    Node *ident = term();
    return new_node('=', ident, new_node('-', ident, new_node_num(1)));
  }else if(consume('+')){
    // +2 -> 2
    return term();
  }else if(consume('-')){
    // -2 -> 0-2
    return new_node('-', new_node_num(0), term());
  }else if(consume('~')){
    return new_node('~', term(), NULL);
  }else if(consume(TK_SIZEOF)){
    if(consume('(')){
      Node *n = unary();
      expect2(')', "parse.c : Line.%d\n  ERROR : sizeofの括弧が閉じられていません ", __LINE__);
      return new_node_num(get_sizeof(n));
    }
    return new_node_num(get_sizeof(unary()));
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

Node *assign(){
  Node *n = expr();
  for(;;){
    if(consume('=')){
      n = new_node('=', n, assign());
    }else if(consume(TK_PLUS_EQ)){
      // a+=2; -> a = a + 2;
      // [OK] a+=2; a+=a; [NG] 1+=1
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
    }else{
      return n;
    }
  }
}

Node *block(){
  Vector *v = new_vector();
  if(consume('{')){
    while(!consume('}')){
      push_back(v, stmt());
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
  Node* n = new_node(ND_IF, NULL, NULL);
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
  }else if(consume(TK_BREAK)){
    n = new_node(ND_BREAK, NULL, NULL);
    expect(';');
  }else if(consume(TK_CONTINUE)){
    n = new_node(ND_CONTINUE, NULL, NULL);
    expect(';');
  }else{
    n = assign();
    expect(';');
  }
  return n;
}

void func_body(){
  while(((Token*)tokens->data[pos])->ty != '}'){
    push_back(nodes, stmt());
  }
}

void init_local_env(char *func_name){
  local_env = new_map();
  map_putm(global_env, func_name, local_env);
}

static void walk(Node *n){
  if(n->lhs != NULL) walk(n->lhs);
  if(n->rhs != NULL) walk(n->rhs);

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

void post_parse(){
  for(int i=0; i< nodes->len; i++){
    walk(nodes->data[i]);
  }
}

void toplevel(){
  while(((Token*)tokens->data[pos])->ty != TK_EOF){
    // return-type
    if(!consume(TK_TYPE)){
      error("Line.%d in parse.c : 関数の宣言は型から始める必要があります", __LINE__);
    }

    Token *t = ((Token*)tokens->data[pos]);
    // function-name
    if(!consume(TK_IDENT)){
      error("Line.%d in parse.c : 関数の宣言から始める必要があります", __LINE__);
    }
    
    char *func_name = t->name;
    Node *n = new_node_declare_func(func_name);
    push_back(nodes, n);
    
    // init. (set a local environment)
    init_local_env(t->name);
    
    // args.
    expect('(');
    while(((Token*)tokens->data[pos])->ty != ')'){
      // if(!consume(TK_TYPE)){
      //  error("Line.%d in parse.c : 仮引数は型から始める必要があります", __LINE__);
      // }
      assume(TK_TYPE);
      term();
      consume(',');
      // todo : refine.
      n->arg_num++;
    }
    expect(')');

    // function-body.
    expect('{');
    func_body();
    expect('}');    
    push_back(nodes, new_node(ND_FUNC_END, NULL, NULL));
  }
  // todo : Node構築後の作業
  post_parse();
}
