#include <stdint.h>
int factorial(int a, int b) {
  int fact = 1;
  (void)b; // unused
  while (a > 1) {
    fact *= a;
    a--;
  }
  return fact;
}
