#include <stdint.h>
int sum_loop(int a, int b) {
  int sum = 0;
  for (int i = 0; i < a; i++) {
    sum += b;
  }
  return sum;
}
