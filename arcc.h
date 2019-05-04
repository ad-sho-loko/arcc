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
  TK_WHILE,
  TK_ELSE,
  TK_AND,
  TK_OR,
  TK_INC,
  TK_DEC,
  TK_PLUS_EQ,
  TK_MINUS_EQ,
  TK_MUL_EQ,
  TK_DIV_EQ
};

enum{
  ND_NUM = 256,
  ND_EQL,
  ND_NEQ,
  ND_LE,
  ND_GE,
  ND_IDENT,
  ND_RETURN,
  ND_EOF,
  ND_IF,
  ND_FOR,
  ND_WHILE,
  ND_ELSE,
  ND_BLOCK,
  ND_AND,
  ND_OR,
  ND_INC,
  ND_DEC,
  ND_PLUS_EQ,
  ND_MINUS_EQ,
  ND_MUL_EQ,
  ND_DIV_EQ
};

// A variable-length array.
typedef struct {
  void **data;
  int cap;
  int len;
} Vector;

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
  struct Node *cond;
  struct Node *then;
  struct Node *els;
  Vector *items;
  int val;
  char* name;
} Node;

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
void debug_vector_nodes(Vector *v);
char* stringfy_token(int tkn_kind);
char* stringfy_node(int node_kind);

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

extern Vector *nodes;
extern Vector *tokens;
extern Map *map;
