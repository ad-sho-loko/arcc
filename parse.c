#include "arcc.h"

static int pos = 0;

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
  map_puti(map, name, (map_len(map) + 1) * 4);
  return n;
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
    error("expected=%c, but accutal=%c in parse.c", ty, actual);
    exit(1);
  }
  pos++;
}

Node* term(){
  if(consume('(')){
    Node *n = assign();
    if(!consume(')')){
      error("unexpected branket `)`");
      exit(1);
    }
    return n;
  }

  Token *tkn = ((Token*)(tokens->data[pos++]));
  if(tkn->ty == TK_NUM){
    return new_node_num(tkn->val);
  }

  if(tkn->ty == TK_IDENT){
    return new_node_ident(tkn->name);
  }
  
  error("数値でも開きカッコでもないトークンです: %s", ((Token*)(tokens->data[pos]))->input);
  exit(1);
}

Node* unary(){
  if(consume(TK_INC)){
    // ++a -> a += 1; a;
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

Node* relational(){
  Node *n = add();
  for(;;){
    if(consume('<')){
      n = new_node('<', n, add());
    }else if(consume(TK_LE)){
      n = new_node(ND_LE, n, add());
    }else if(consume('>')){
      n = new_node(ND_LE, add(), n);
    }else if(consume(TK_GE)){
      n = new_node(ND_LE, add(), n);
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
  // todo : check correct priority.
  n->init = assign();
  expect(';');
  n->cond = expr();
  expect(';');
  n->last = assign();
  expect(')');
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
  }else{
    n = assign();
    expect(';');
  }
  return n;
}

void program(){
  while(((Token*)tokens->data[pos])->ty != TK_EOF){
    push_back(nodes, stmt());
  }
}
