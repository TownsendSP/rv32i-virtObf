#include <stdint.h>
int shl(int a, int b) { return a << (b & 0x1F); }
