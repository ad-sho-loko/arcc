#include <stdlib.h>

enum{
  TK_NUM = 256,
  TK_EQL,
  TK_NEQ,
  TK_LE,
  TK_GE,
  TK_IDENT,
  TK_EOF
};

typedef struct{
  int ty;
  int val;
  char name;
  char *input;
} Token;

typedef struct Node{
  int ty;
  struct Node *lhs;
  struct Node *rhs;  
  int val;
  char name;
} Node;

// A variable-length array.
typedef struct {
  void **data;
  int cap;
  int len;
} Vector;

Vector *new_vector();
void push_back(Vector *v, void* elm);

void error(char* fmt, ...);
Node *add();
Node *mul();
Node *term();
Node *unary();
Node *equality();
Node *assign();
Node *stmt();
void program();
Vector *tokenize(char *p);
void gen(Node *n);
  
extern Node *codes[100];
extern Vector *tokens;
