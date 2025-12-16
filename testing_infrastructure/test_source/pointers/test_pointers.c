#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

extern int32_t doOperation(int32_t a, int32_t b);

int main(int argc, char *argv[]) {
  if (argc < 3)
    return 1;
  int32_t a = atoi(argv[1]);
  int32_t b = atoi(argv[2]);

  int32_t result = doOperation(a, b);
  printf("%d\n", result);
  return 0;
}
