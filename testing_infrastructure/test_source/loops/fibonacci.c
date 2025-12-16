#include <stdint.h>
int fibonacci(int a, int b) {
  (void)b;
  if (a <= 1) return a;
  int prev = 0;
  int curr = 1;
  int i = 2;
  while (i <= a) {
    int next = prev + curr;
    prev = curr;
    curr = next;
    i++;
  }
  return curr;
}
