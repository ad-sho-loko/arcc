#include <stdlib.h>

enum{
  TK_NUM = 256,
  TK_EQL,
  TK_NEQ,
  TK_LE,
  TK_GE,
  TK_IDENT,
  TK_RETURN,
  TK_EOF,
  TK_IF,
  TK_FOR,
  TK_WHILE
};

typedef struct{
  int ty;
  int val;
  char *name;
  char *input;
} Token;

typedef struct Node{
  int ty;
  struct Node *lhs;
  struct Node *rhs;  
  int val;
  char* name;
} Node;

// A variable-length array.
typedef struct {
  void **data;
  int cap;
  int len;
} Vector;

Vector *new_vector();
void push_back(Vector *v, void* elm);

// A simple map
typedef struct{
  Vector* keys;
  Vector* values;
} Map;

Map *new_map();
int map_geti(Map* m, char *key);
void map_puti(Map* m, char* key, int value);
int map_len(Map* m);

void error(char* fmt, ...);
void out(char* code);
void printd(char *s);
void debug_vector_token(Vector *v);
char* stringfy_token(int tkn_kind);

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
extern Map *map;
