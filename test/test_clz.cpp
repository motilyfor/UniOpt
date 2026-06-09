#include <iostream>
#include <cstring>
#include <cassert>

#include "opt_bit.hpp"
#include "test_base.hpp"

int test_clz_1()
{
    int failures = 0;
    failures += CHECK_EQUAL(32, opt_clz(0x00000000), "clz(0)");
    return failures;
}

int test_clz_2()
{
    int failures = 0;
    failures += CHECK_EQUAL(0, opt_clz(0x80000000), "clz(MSB)");
    return failures;
}
int test_clz_3()
{
    int failures = 0;
    failures += CHECK_EQUAL(8, opt_clz(0x00800000), "clz(0x00800000)");
    return failures;
}

int main()
{
    const int f = test_clz_1();
    if (f == 0) std::cout << "test_clz_1 passed" << std::endl;
    else std::cout << "test_clz_1 failures: " << f << std::endl;

    const int f2 = test_clz_2();
    if (f2 == 0) std::cout << "test_clz_2 passed" << std::endl;
    else std::cout << "test_clz_2 failures: " << f2 << std::endl;

    const int f3 = test_clz_3();
    if (f3 == 0) std::cout << "test_clz_3 passed" << std::endl;
    else std::cout << "test_clz_3 failures: " << f3 << std::endl;

    return f;
}