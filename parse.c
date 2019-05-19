#include "arcc.h"

static int pos = 0;
static Map *local_env;

Node *new_node(int ty, Node *lhs, Node *rhs){
  Node* n = malloc(sizeof(Node));
  n->ty = ty;
  n->lhs = lhs;
  n->rhs = rhs;
  return n;
}

Node *new_node_num(int val){
  Node* n = malloc(sizeof(Node));
  n->ty = ND_NUM;
  n->val = val;
  return n;
}

Node *new_node_ident(char* name){
  Node *n = malloc(sizeof(Node));
  n->ty = ND_IDENT;
  n->name = name;
  map_puti(local_env, name, (map_len(local_env) + 1) * 8);
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

void assume(int ty){
  int actual = ((Token*)(tokens->data[pos]))->ty;
  if(actual != ty){
    error("Line.%d in parse.c : expected=%c but accutal=%c %d  in parse.c", __LINE__, ty, actual, actual);
  }
}

int consume(int ty){
  if(((Token*)tokens->data[pos])->ty != ty)
    return 0;
  pos++;
  return 1;
}

void expect(int ty){
  int actual = ((Token*)(tokens->data[pos]))->ty;
  if(actual != ty){
    error("Line.%d in parse.c : expected=%c but accutal=%c %d  in parse.c", __LINE__, ty, actual, actual);
  }
  pos++;
}

Node* ident(Token *tkn){
  // WHEN calling function
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
  
  // WHEN variable : 一度定義されていないとエラーになる.
  if(map_geti(local_env, tkn->name) == (int)NULL){
    error("Line.%d in parse.c : 定義されていない変数です : %s", __LINE__, tkn->name);
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
      error("Line%d in parse.c : unexpected branket `)`", __LINE__);
    }
    return n;
  }

  Token *tkn = ((Token*)(tokens->data[pos]));
  pos++;

  if(tkn->ty == TK_NUM){
    return new_node_num(tkn->val);
  }

  // The progress of implemention.
  // [o] int a;
  // [x] int a = 1;
  // [x] int a, b;
  // [x] int a = 1, b;
  // [x] int a = 1, b = 2;
  if(tkn->ty == TK_TYPE){
    if(((Token*)(tokens->data[pos]))->ty != TK_IDENT){
      error("Line.%d 型宣言のあとは変数でなければいけません : parse.c ", __LINE__);
    }

    Token *t = ((Token*)(tokens->data[pos]));
    Node *n = new_node_ident(t->name);
    pos++;
    return n;
  }

  if(tkn->ty == TK_ADR){
    Token *t = ((Token*)(tokens->data[pos]));

    if(t->ty != TK_IDENT){
      error("アンパサンドのあとは必ず変数です");
    }

    // 一度定義されていないとエラーになる.
    if(map_geti(local_env, t->name) == (int)NULL){
      error("Line.%d in parse.c : 定義されていない変数です : %s", __LINE__, t->name);
    }
    pos++;
    Node *n = new_node(ND_ADR, NULL, NULL);
    n->name = t->name;
    return n;
  }

  if(tkn->ty == TK_PTR){
    Token *t = ((Token*)(tokens->data[pos]));

    if(t->ty != TK_IDENT){
      error("ポインタのあとは必ず変数です");
    }

    // 一度定義されていないとエラーになる.
    if(map_geti(local_env, t->name) == (int)NULL){
      error("Line.%d in parse.c : 定義されていない変数です : %s", __LINE__, t->name);
    }

    pos++;
    Node *n = new_node(ND_PTR, NULL, NULL);
    n->name = t->name;
    return n;
  }
  
  if(tkn->ty == TK_IDENT){
    return ident(tkn);
  }
  
  error("Line.%d in parse.c : 数値でも開きカッコでもないトークンです: %s", __LINE__  ,((Token*)(tokens->data[pos]))->input);
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

Node *expr(){
  Node *n = equality();
  for(;;){
    if(consume(TK_AND)){
      n = new_node(ND_AND, n, equality());
    }else if(consume(TK_OR)){
      n = new_node(ND_OR, n, equality());
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
      if(!consume(TK_TYPE)){
        error("Line.%d in parse.c : 仮引数は型から始める必要があります", __LINE__);
      }
      Token *now = (Token*)tokens->data[pos];
      push_back(nodes, new_node_ident(now->name));
      expect(TK_IDENT);
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
}
