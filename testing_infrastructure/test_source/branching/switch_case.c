#include <stdint.h>
int switch_case(int a, int b) {
  switch (a) {
  case 0:
    return b + 1;
  case 1:
    return b + 2;
  case 2:
    return b + 3;
  default:
    return b + 4;
  }
}
