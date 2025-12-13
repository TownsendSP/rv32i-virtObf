int doOperation(int a, int b) {
    int c = 7*a;
    int *d = &b;
    int e = *d + 11;
    if (c > e) {
        c = c - 3;
    } else {
        c = c + 3;
    }
    return c + e;
}

int main() {
    int output = doOperation(5, 3);
    return output;
}
