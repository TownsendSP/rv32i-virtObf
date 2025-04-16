//
// Created by tgsp on 4/15/25.
//
#include <stdio.h>
int doOperation(int a, int b) {
    return a + b;
}

int main() {
    int output = doOperation(5, 3);
    printf("isitworking: %d\n", output);
    return output;
}
