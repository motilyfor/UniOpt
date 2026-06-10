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

int test_vec_add_i32()
{
    int failures = 0;
    const size_t n = 11;
    int32_t x[n] = {0, -1, 1, -123456, 123456, INT32_MIN, INT32_MAX, -1000000000, 1000000000, -2147483647, 2147483646};
    int32_t y[n] = {1, 1, -1, 123456, -123456, 1, -1, 1000000000, -1000000000, 2147483646, -2147483647};
    int32_t dst_scalar[n];
    int32_t dst_vec[n];

    for (size_t i = 0; i < n; ++i) dst_scalar[i] = opt_add_i32(x[i], y[i]);
    opt_vec_add_i32(dst_vec, x, y, n);

    for (size_t i = 0; i < n; ++i) {
        failures += CHECK_EQUAL(dst_scalar[i], dst_vec[i], "opt_vec_add_i32 mismatch");
    }

    return failures;
}

int test_vec_add_i16()
{
    int failures = 0;
    const size_t n = 17;
    int16_t x[n] = {0, -1, 1, -1234, 1234, INT16_MIN, INT16_MAX, -327, 327, -1000, 1000, -2, 2, -32767, 32766, -15, 15};
    int16_t y[n] = {1, 1, -1, 1234, -1234, 1, -1, 327, -327, 1000, -1000, 2, -2, 32766, -32767, 15, -15};
    int16_t dst_scalar[n];
    int16_t dst_vec[n];

    for (size_t i = 0; i < n; ++i) dst_scalar[i] = opt_add_i16(x[i], y[i]);
    opt_vec_add_i16(dst_vec, x, y, n);

    for (size_t i = 0; i < n; ++i) {
        failures += CHECK_EQUAL(dst_scalar[i], dst_vec[i], "opt_vec_add_i16 mismatch");
    }

    return failures;
}

int main()
{
    const int s = test_add_i32();
    if (s == 0) std::cout << "test_add_i32 passed" << std::endl;
    else std::cout << "test_add_i32 failures: " << s << std::endl;

    const int v16 = test_vec_add_i16();
    if (v16 == 0) std::cout << "test_vec_add_i16 passed" << std::endl;
    else std::cout << "test_vec_add_i16 failures: " << v16 << std::endl;

    const int v32 = test_vec_add_i32();
    if (v32 == 0) std::cout << "test_vec_add_i32 passed" << std::endl;
    else std::cout << "test_vec_add_i32 failures: " << v32 << std::endl;

    return s + v16 + v32;
}