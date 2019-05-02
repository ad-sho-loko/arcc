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
  n->ty = TK_NUM;
  n->val = val;
  return n;
}

Node *new_node_ident(char name){
  Node *n = malloc(sizeof(Node));
  n->ty = TK_IDENT;
  n->name = name;
  return n;
}

int consume(int ty){
  if(((Token*)tokens->data[pos])->ty != ty)
    return 0;
  pos++;
  return 1;
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
    return new_node_ident(tkn->val);
  }
  
  error("数値でも開きカッコでもないトークンです: %s", ((Token*)(tokens->data[pos]))->input);
  exit(1);
}

Node* unary(){
  if(consume('+')){
    return term();
  }else if(consume('-')){
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
      n = new_node(TK_LE, n, add());
    }else if(consume('>')){
      n = new_node(TK_LE, add(), n);
    }else if(consume(TK_GE)){
      n = new_node(TK_LE, add(), n);
    }else{
      return n;      
    }
  }
}

Node *equality(){
  Node *n = relational();
  for(;;){
    if(consume(TK_EQL)){
      n = new_node(TK_EQL, n, relational());
    }else if(consume(TK_NEQ)){
      n = new_node(TK_NEQ, n, relational());
    }else{
      return n;
    }
  }
}

Node *assign(){
  Node *n = equality();
  while(consume('='))
    n = new_node('=', n, assign());
  return n;
}

Node *stmt(){
  Node* n;
  if(consume(TK_RETURN)){
    n = malloc(sizeof(Node));
    n->ty = TK_RETURN;
    n->lhs = assign();
  }else{
    n = assign();
  }
  
  if(!consume(';')){
    error(";ではないトークンです %s", ((Token*)tokens->data[pos])->input);
    exit(1);
  }
  return n;
}

void program(){
  int i = 0;
  while(((Token*)tokens->data[pos])->ty != TK_EOF){
    codes[i++] = stmt();
  }
  codes[i] = NULL;
}

