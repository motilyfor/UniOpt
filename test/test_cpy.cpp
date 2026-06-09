#include <iostream>
#include <vector>
#include <random>
#include <cstring>
#include <cassert>

// Include the implementation inside the expected nested namespace.
#include "../libuniopt/inc/opt_cpy.hpp"

int main() {
    std::mt19937_64 rng(12345);
    std::uniform_int_distribution<unsigned long long> dist;

    std::vector<std::size_t> sizes = {1,2,15,16,31,32,63,64,127,128,255,256,1024,4096,8192,16384};

    for (auto size : sizes) {
        void* src_ptr = nullptr;
        void* dst1_ptr = nullptr;
        void* dst2_ptr = nullptr;
        // allocate 32-byte aligned buffers to satisfy AVX aligned intrinsics
        const std::size_t align = 32;
        if (posix_memalign(&src_ptr, align, size) != 0) src_ptr = nullptr;
        if (posix_memalign(&dst1_ptr, align, size) != 0) dst1_ptr = nullptr;
        if (posix_memalign(&dst2_ptr, align, size) != 0) dst2_ptr = nullptr;
        if (!src_ptr || !dst1_ptr || !dst2_ptr) {
            std::cerr << "Allocation failed for size " << size << "\n";
            free(src_ptr); free(dst1_ptr); free(dst2_ptr);
            return 3;
        }

        unsigned char* srcb = reinterpret_cast<unsigned char*>(src_ptr);
        for (std::size_t i = 0; i < size; ++i) srcb[i] = static_cast<unsigned char>(dist(rng) & 0xFF);

        // call opt_cpy (qualified namespace)
        opt_cpy(dst1_ptr, src_ptr, size);

        // fallback memcpy
        std::memcpy(dst2_ptr, src_ptr, size);

        if (std::memcmp(dst1_ptr, dst2_ptr, size) != 0) {
            std::cerr << "Mismatch for size " << size << "\n";
            free(src_ptr); free(dst1_ptr); free(dst2_ptr);
            return 2;
        }

        free(src_ptr);
        free(dst1_ptr);
        free(dst2_ptr);
    }

    std::cout << "All tests passed." << std::endl;
    return 0;
}
