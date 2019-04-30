#include <stdlib.h>

// A variable-length array.
typedef struct {
  void **data;
  int32_t cap;
  int32_t len;
} Vector;

// hash map.
