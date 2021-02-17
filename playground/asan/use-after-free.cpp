// Example from https://github.com/google/sanitizers/wiki/AddressSanitizer#using-addresssanitizer

#include <stdlib.h>

int main() {
  char *x = (char*)malloc(10 * sizeof(char*));
  free(x);
  return x[5];
}
