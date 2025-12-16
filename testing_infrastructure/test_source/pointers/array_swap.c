#include <stdint.h>
int array_swap(int a, int b) {
  int arr[2];
  arr[0] = a;
  arr[1] = b;

  int temp = arr[0];
  arr[0] = arr[1];
  arr[1] = temp;

  return arr[0]; // should be b
}
