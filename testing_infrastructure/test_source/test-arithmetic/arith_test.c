/*
 * arith_test.c - Test virtualized Arithmetic function execution
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern int32_t doOperation(int32_t a, int32_t b);

int main(int argc, char *argv[]) {
    int32_t a = atoi(argv[1]);
    int32_t b = atoi(argv[2]);

    int32_t got = doOperation(a, b);

    printf("doOperation(%d, %d) = %d\n", a, b, got);
    return got;
}
