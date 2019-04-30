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
  char *input;
} Token;

typedef struct Node{
  int ty;
  struct Node *lhs;
  struct Node *rhs;  
  int val;
} Node;

extern Token tokens[100];
extern Node *codes[100];

void error(char* fmt, ...);
Node *add();
Node *mul();
Node *term();
Node *unary();
Node *equality();
Node *assign();
Node *stmt();
void program();
void tokenize(char *p);
void gen(Node *n);

// A variable-length array.
typedef struct {
  void **data;
  int32_t cap;
  int32_t len;
} Vector;

// hash map.
