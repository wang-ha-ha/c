//
// Created by wxl on 23-11-22.
//

#include <cstdio>
#include <cstdlib>

void segv() {
    int *ptr = (int*)0x12345678;
    printf("%d\n", *ptr);
}

void fpe() {
    printf("%.2f", 1.0f / 0);
}

int main() {
    segv();
    return 0;
}
