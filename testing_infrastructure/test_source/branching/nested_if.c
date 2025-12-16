#include <stdint.h>
int nested_if(int a, int b) {
  if (a > 0) {
    if (b > 0)
      return 1;
    else
      return 2;
  } else {
    if (b > 0)
      return 3;
    else
      return 4;
  }
}
