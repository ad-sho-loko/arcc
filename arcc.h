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
  TK_DIV_EQ,
  TK_ELSE_IF,
  TK_BREAK,
  TK_CONTINUE,
  TK_INT,
  TK_PTR,
  TK_ADR,
  TK_TYPE,
  TK_REM_EQ,
  TK_LSHIFT,
  TK_RSHIFT,
  TK_LSHIFT_EQ,
  TK_RSHIFT_EQ,
  TK_SIZEOF,
  TK_AND_EQ,
  TK_OR_EQ,
  TK_XOR_EQ,
  TK_DO
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
  ND_FUNC,
  ND_DEC_FUNC,
  ND_FUNC_END,
  ND_BREAK,
  ND_CONTINUE,
  ND_INT,
  ND_PTR,
  ND_ADR,
  ND_TYPE,
  ND_LSHIFT,
  ND_RSHIFT,
  ND_DO_WHILE,
  ND_DUMMY
};

typedef struct Type{
  enum {INT, PTR, ARRAY} ty;
  struct Type *ptr_of;
  int array_size;
}Type;

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
  Type *type; // for pointer.
} Token;

typedef struct Node{
  int ty;
  struct Node *lhs;
  struct Node *rhs;
  struct Node *cond;
  struct Node *then;
  struct Node *els;
  struct Node *init; // for(xxx; ; )
  struct Node *last; // for(;; xxx)
  Vector *items;
  int val;
  int arg_num;
  int cnt;  // for ***x
  char* name;
} Node;

typedef struct{
  Type *type;
  char* name;
  int pos;
} Var;

// methods for vector.
Vector *new_vector();
void push_back(Vector *v, void* elm);

// A simple map
typedef struct{
  Vector* keys;
  Vector* values;
} Map;

// methods for map.
Map *new_map();
Var *map_getv(Map* m, char *key);
void map_putv(Map* m, char* key, Var* value);
Map *map_getm(Map* m, char *key);
void map_putm(Map* m, char* key, Map* value);
int map_indexOf(Map* m, char *key);
int map_len(Map* m);
int map_contains(Map* m, char* key);

typedef struct{
  int pos;
  Vector* items;
}Stack;

Stack *new_stack();
void stack_push(Stack *stack, void* v);
void *stack_pop(Stack *stack);
void *stack_peek(Stack *stack);

// global variables.
extern Vector *nodes;
extern Vector *tokens;
extern Map *global_env;

void error(char* fmt, ...);
void out(char* code);
void outf(char* fmt, ...);
void outd(char* code);
void printd(char* fmt, ...);
int do_align(int x, int align);
void debug_vector_token(Vector *v);
void debug_vector_nodes(Vector *v);
void debug_variable_table();
char* stringfy_token(int tkn_kind);
char* stringfy_node(int node_kind);

// token.c
Vector *tokenize(char *p);

// parse.c
Node *add();
Node *mul();
Node *term();
Node *unary();
Node *equality();
Node *assign();
Node *stmt();
void func_body();
void toplevel();

// gen.c
void gen(Node *n);
void gen_top();
