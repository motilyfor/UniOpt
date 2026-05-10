#pragma once

#include <iostream>

template<typename T, typename U>
int CHECK_EQUAL(const T& expected, const U& actual, const char* message = "")
{
    if (!(expected == actual)) {
        std::cerr << "Check failed";
        if (message && message[0]) std::cerr << ": " << message;
        std::cerr << " | expected=" << expected << " actual=" << actual << "\n";
        return 1;
    }
    return 0;
}