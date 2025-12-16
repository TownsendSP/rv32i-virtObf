/*
 * arith_test.c - Test virtualized Arithmetic function execution
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern int doOperation(int a, int b);

int main(int argc, char *argv[]) {
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);

    int got = doOperation(a, b);

    printf("doOperation(%d, %d) = %d\n", a, b, got);
    return got;
}
