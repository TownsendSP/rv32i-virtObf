#include <stdint.h>
int ptr_arithmetic(int a, int b) {
  int arr[5] = {a, b, a + b, a - b, a * b};
  int *ptr = arr;
  ptr += 2; // arr[2] which is a+b
  return *ptr;
}
