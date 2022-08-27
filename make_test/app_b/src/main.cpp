#include "lib_a.h"
#include "lib_b.h"
#include "app_b.h"
#include <iostream>

int main(int argc, char **argv)
{
    int a = lib_a();
    int b = m_fun();

    a = lib_b(a,b);

    printf("a = %d, b = %d\n", a, b);
    std::cout << "cpp test" << std::endl;

    return 0;
}