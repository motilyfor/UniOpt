#include <iostream>
#include <vector>
#include <cstring>
#include "../libuniopt/inc/opt_abs.hpp"

void dump_i16(const char* name, const int16_t* a, size_t n){
    std::cout<<name<<": ";
    for(size_t i=0;i<n;i++) std::cout<<a[i]<<" ";
    std::cout<<"\n";
}

void dump_i32(const char* name, const int32_t* a, size_t n){
    std::cout<<name<<": ";
    for(size_t i=0;i<n;i++) std::cout<<a[i]<<" ";
    std::cout<<"\n";
}

int main(){
    {
        std::cout<<"Test i16 abs\n";
        std::vector<int16_t> src = {0, -1, 1, -1234, 1234, INT16_MIN, INT16_MAX, -327, 327, -1000, 1000, -2, 2, -32767, 32766, -15, 15};
        size_t n = src.size();
        std::vector<int16_t> dst_c(n), dst_vec(n);
        opt_vec_i16(dst_c.data(), src.data(), n);
        opt_vec_i16(dst_vec.data(), src.data(), n);
        dump_i16("src", src.data(), n);
        dump_i16("c  ", dst_c.data(), n);
        dump_i16("simd", dst_vec.data(), n);
        bool ok=true;
        for(size_t i=0;i<n;i++) if(dst_c[i]!=dst_vec[i]){ ok=false; std::cout<<"mismatch i16 at "<<i<<": c="<<dst_c[i]<<" simd="<<dst_vec[i]<<"\n"; }
        std::cout<<"i16 OK="<<ok<<"\n\n";
    }

    {
        std::cout<<"Test i32 abs\n";
        std::vector<int32_t> src = {0, -1, 1, -123456, 123456, INT32_MIN, INT32_MAX, -1000000000, 1000000000, -2147483647, 2147483646};
        size_t n = src.size();
        std::vector<int32_t> dst_c(n), dst_vec(n);
        opt_vec_i32(dst_c.data(), src.data(), n);
        opt_vec_i32(dst_vec.data(), src.data(), n);
        dump_i32("src", src.data(), n);
        dump_i32("c  ", dst_c.data(), n);
        dump_i32("simd", dst_vec.data(), n);
        bool ok=true;
        for(size_t i=0;i<n;i++) if(dst_c[i]!=dst_vec[i]){ ok=false; std::cout<<"mismatch i32 at "<<i<<": c="<<dst_c[i]<<" simd="<<dst_vec[i]<<"\n"; }
        std::cout<<"i32 OK="<<ok<<"\n\n";
    }
    return 0;
}
