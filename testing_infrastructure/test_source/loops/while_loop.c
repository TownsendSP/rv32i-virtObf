#include <stdint.h>
int while_loop(int a, int b) {
  int count = 0;
  while (a < b) {
    a++;
    count++;
  }
  return count;
}
