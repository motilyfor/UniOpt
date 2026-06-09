#include <iostream>
#include <cmath>
#include <iomanip>

#include "test_base.hpp"
#include "opt_divide.hpp"

// Wrapper to call opt_divide_q31 with specific number of iterations
// We cheat by redefining or using a local copy since the original doesn't take iter count
void opt_divide_q31_fixed_iter(int32_t& frac, int32_t& exp, int32_t x, int32_t y, int iterations)
{
    if (y == 0) {
        frac = 0x7FFFFFFF;
        exp = 0;
        return;
    }

    int32_t sign = (x ^ y) >> 31;
    uint32_t ux = (x < 0) ? -x : (uint32_t)x;
    uint32_t uy = (y < 0) ? -y : (uint32_t)y;

    int32_t sx = opt_clz(ux);
    int32_t sy = opt_clz(uy);
    ux <<= sx;
    uy <<= sy;

    uint32_t z = (uint32_t)0xBB67A0F9 - uy;

    for(int i = 0; i < iterations; i++) {
        int32_t e = 0x40000000 - opt_sat_q31((((int64_t)uy * z) + (1L << 30)) >> 31);
        e <<= 1;
        z = opt_add_i32(z, opt_sat_q31((((int64_t)z * e) + (1L << 30)) >> 31));
    }

    // frac = opt_mult_q31(z, ux);
    uint64_t full_res = ((uint64_t)z * ux + (1ULL << 30)) >> 31;
    frac = (int32_t)full_res;

    frac = sign ? -frac : frac;
    exp = sy - sx + 1;
}

template<typename T, typename U>
int CHECK_NEAR(const T& expected, const U& actual, double tolerance, const char* message = "")
{
    double diff = std::abs((double)expected - (double)actual);
    if (diff > tolerance) {
        std::cerr << "Check failed";
        if (message && message[0]) std::cerr << ": " << message;
        std::cerr << " | expected=" << expected << " actual=" << actual << " diff=" << diff << " tolerance=" << tolerance << "\n";
        return 1;
    }
    return 0;
}

int test_divide_precision()
{
    int failures = 0;
    int32_t frac = 0;
    int32_t exp = 0;

    // Test values: 1.0/2.0, 1.0/3.0, 100/7.0, etc.
    // Since it's Q31, we can represent real values as double for comparison.
    // Result of opt_divide_q31 is approximately (frac / 2^31) * 2^exp
    
    struct {
        int32_t x;
        int32_t y;
    } test_cases[] = {
        {0x40000000, 0x40000000},  // 1 / 1
        {0x40000000, 0x20000000},  // 1 / 0.5 = 2
        {0x20000000, 0x40000000},  // 0.5 / 1 = 0.5
        {-0x40000000, 0x40000000}, // -1 / 1 = -1
        {0x40000000, -0x40000000}, // 1 / -1 = -1
        {-1000, 3},                // -333.333...
        {1000, -3},                // -333.333...
        {-12345, -6789},           // 1.818...
        {0x7FFFFFFF, 1},           // max / 1
        {1, 0x7FFFFFFF},           // 1 / max
    };

    std::cout << "\n--- Precision Test Results ---" << std::endl;
    std::cout << std::setw(12) << "x" << std::setw(12) << "y" 
              << std::setw(15) << "Expected" << std::setw(15) << "Actual" 
              << std::setw(15) << "Error" << std::endl;

    for (auto& tc : test_cases) {
        opt_divide_q31(frac, exp, tc.x, tc.y);
        
        double expected = (double)tc.x / (double)tc.y;
        double actual = ((double)frac / 2147483648.0) * std::pow(2.0, exp);
        double error = std::abs(expected - actual);

        std::cout << std::setw(12) << tc.x << std::setw(12) << tc.y 
                  << std::setw(15) << expected << std::setw(15) << actual 
                  << std::setw(15) << error << std::endl;

        // Tolerance: 1e-7 relative error is usually acceptable
        // Using abs() for tolerance check to handle negative 'expected'
        failures += CHECK_NEAR(expected, actual, std::abs(expected) * 1e-6, "Precision check");
    }
    std::cout << "------------------------------\n" << std::endl;

    return failures;
}

int test_divide_q31_1()
{
    int failures = 0;
    int32_t frac = 0;
    int32_t exp = 0;

    opt_divide_q31(frac, exp, 123456789, 0);
    failures += CHECK_EQUAL(0x7FFFFFFF, frac, "divide_q31 divide by zero frac");
    failures += CHECK_EQUAL(0, exp, "divide_q31 divide by zero exp");

    return failures;
}

int test_divide_q31_2()
{
    int failures = 0;
    int32_t frac = 0;
    int32_t exp = 0;

    opt_divide_q31(frac, exp, 0x40000000, 0x40000000);
    failures += CHECK_EQUAL(1, exp, "divide_q31 exponent when x==y");

    opt_divide_q31(frac, exp, 0x20000000, 0x40000000);
    failures += CHECK_EQUAL(0, exp, "divide_q31 exponent when x<y");

    opt_divide_q31(frac, exp, 0x40000000, 0x20000000);
    failures += CHECK_EQUAL(2, exp, "divide_q31 exponent when x>y");

    return failures;
}

int test_divide_precision_comparison()
{
    int32_t frac3 = 0, frac4 = 0;
    int32_t exp3 = 0, exp4 = 0;

    struct {
        int32_t x;
        int32_t y;
        const char* name;
    } test_cases[] = {
        {1000, 3, "1000 / 3"},
        {12345, 6789, "12345 / 6789"},
        {0x7FFFFFFF, 12345, "MAX / 12345"},
        {1, 0x7FFFFFFF, "1 / MAX"},
        {0x40000000, 0x30000000, "0.5 / 0.375"},
    };

    std::cout << "\n--- Iteration Comparison (3 vs 4) ---" << std::endl;
    std::cout << std::left << std::setw(20) << "Case" 
              << std::setw(20) << "3-Iter Error" 
              << std::setw(20) << "4-Iter Error" 
              << std::setw(20) << "Improvement" << std::endl;

    for (auto& tc : test_cases) {
        opt_divide_q31_fixed_iter(frac3, exp3, tc.x, tc.y, 3);
        opt_divide_q31_fixed_iter(frac4, exp4, tc.x, tc.y, 4);
        
        double expected = (double)tc.x / (double)tc.y;
        double actual3 = ((double)frac3 / 2147483648.0) * std::pow(2.0, exp3);
        double actual4 = ((double)frac4 / 2147483648.0) * std::pow(2.0, exp4);
        
        double error3 = std::abs(expected - actual3);
        double error4 = std::abs(expected - actual4);

        std::cout << std::left << std::setw(20) << tc.name
                  << std::left << std::setw(20) << error3
                  << std::left << std::setw(20) << error4
                  << std::left << std::setw(20) << (error3 - error4) << std::endl;
    }
    std::cout << "--------------------------------------\n" << std::endl;

    return 0;
}

int main()
{
    test_divide_precision_comparison();
    const int f1 = test_divide_q31_1();
    if (f1 == 0) std::cout << "test_divide_q31_1 passed" << std::endl;
    else std::cout << "test_divide_q31_1 failures: " << f1 << std::endl;

    const int f2 = test_divide_q31_2();
    if (f2 == 0) std::cout << "test_divide_q31_2 passed" << std::endl;
    else std::cout << "test_divide_q31_2 failures: " << f2 << std::endl;

    const int f3 = test_divide_precision();
    if (f3 == 0) std::cout << "test_divide_precision passed" << std::endl;
    else std::cout << "test_divide_precision failures: " << f3 << std::endl;

    return f1 + f2 + f3;
}