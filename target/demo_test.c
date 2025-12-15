/*
 * demo_main.c - Test virtualized function execution
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

extern int32_t doOperation(int32_t a, int32_t b);

/* Reference: c = 7*a, e = b+11, if(c>e) c-=3 else c+=3, return c+e */
int32_t reference(int32_t a, int32_t b) {
    int32_t c = 7 * a;
    int32_t e = b + 11;
    c += (c > e) ? -3 : 3;
    return c + e;
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <a> <b>\n", argv[0]);
        return 1;
    }

    int32_t a = atoi(argv[1]);
    int32_t b = atoi(argv[2]);

    int32_t got = doOperation(a, b);
    int32_t want = reference(a, b);

    printf("doOperation(%d, %d) = %d %s\n", 
           a, b, got, (got == want) ? "OK" : "FAIL");

    return (got == want) ? 0 : 1;
}
