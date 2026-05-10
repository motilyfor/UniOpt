#include "test_base.hpp"
#include "../libuniopt/inc/opt_add.hpp"

int test_add_i32()
{
    int failures = 0;
    failures += CHECK_EQUAL(5, opt_add_i32(2, 3), "2 + 3");
    failures += CHECK_EQUAL(-1, opt_add_i32(2, -3), "2 + (-3)");
    failures += CHECK_EQUAL(-5, opt_add_i32(-2, -3), "-2 + (-3)");
    failures += CHECK_EQUAL(INT32_MAX, opt_add_i32(INT32_MAX, 1), "INT32_MAX + 1 (overflow)");
    failures += CHECK_EQUAL(INT32_MIN, opt_add_i32(INT32_MIN, -1), "INT32_MIN + (-1) (underflow)");
    return failures;
}