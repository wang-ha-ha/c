#include "lib_a.h"
#include "lib_b.h"
#include "app_a.h"
#include <stdio.h>

int main(int argc, char **argv)
{
    int a = lib_a();
    int b = m_fun();

    a = lib_b(a,b);

    printf("a = %d, b = %d\n", a, b);

    return 0;
}