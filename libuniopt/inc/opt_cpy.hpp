#pragma once

#include <cassert>
#include <cstddef>
#include <cstdint>

#include "opt_base.hpp"

#if defined(DEBUG)
#    define static
#endif

static INLINE void opt_cpy_16(void* dst, const void* src)
{
#if defined(OPT_AVX)
    __m128i m0 = _mm_loadu_si128(((const __m128i*)src) + 0);
    _mm_storeu_si128(((__m128i*)dst) + 0, m0);
#elif defined(OPT_NEON)
    uint8x16_t v0 = vld1q_u8((const uint8_t*)src + 0);
    vst1q_u8((uint8_t*)dst + 0, v0);
#endif
}

static INLINE void opt_cpy_32(void* dst, const void* src)
{
#if defined(OPT_AVX)
    __m256i m0 = _mm256_loadu_si256(((const __m256i*)src) + 0);
    _mm256_storeu_si256(((__m256i*)dst) + 0, m0);
#elif defined(OPT_NEON)
    uint8x16_t v0 = vld1q_u8((const uint8_t*)src + 0);
    uint8x16_t v1 = vld1q_u8((const uint8_t*)src + 16);
    vst1q_u8((uint8_t*)dst + 0, v0);
    vst1q_u8((uint8_t*)dst + 16, v1);
#endif
}

static INLINE void opt_cpy_64(void* dst, const void* src)
{
#if defined(OPT_AVX)
    __m256i m0 = _mm256_loadu_si256(((const __m256i*)src) + 0);
    __m256i m1 = _mm256_loadu_si256(((const __m256i*)src) + 1);
    _mm256_storeu_si256(((__m256i*)dst) + 0, m0);
    _mm256_storeu_si256(((__m256i*)dst) + 1, m1);
#elif defined(OPT_NEON)
    uint8x16_t v0 = vld1q_u8((const uint8_t*)src + 0);
    uint8x16_t v1 = vld1q_u8((const uint8_t*)src + 16);
    uint8x16_t v2 = vld1q_u8((const uint8_t*)src + 32);
    uint8x16_t v3 = vld1q_u8((const uint8_t*)src + 48);
    vst1q_u8((uint8_t*)dst + 0, v0);
    vst1q_u8((uint8_t*)dst + 16, v1);
    vst1q_u8((uint8_t*)dst + 32, v2);
    vst1q_u8((uint8_t*)dst + 48, v3);
#endif
}

static INLINE void opt_cpy_128(void* dst, const void* src)
{
#if defined(OPT_AVX)
    __m256i m0 = _mm256_loadu_si256(((const __m256i*)src) + 0);
    __m256i m1 = _mm256_loadu_si256(((const __m256i*)src) + 1);
    __m256i m2 = _mm256_loadu_si256(((const __m256i*)src) + 2);
    __m256i m3 = _mm256_loadu_si256(((const __m256i*)src) + 3);
    _mm256_storeu_si256(((__m256i*)dst) + 0, m0);
    _mm256_storeu_si256(((__m256i*)dst) + 1, m1);
    _mm256_storeu_si256(((__m256i*)dst) + 2, m2);
    _mm256_storeu_si256(((__m256i*)dst) + 3, m3);
#elif defined(OPT_NEON)
    uint8x16_t v0 = vld1q_u8((const uint8_t*)src + 0);
    uint8x16_t v1 = vld1q_u8((const uint8_t*)src + 16);
    uint8x16_t v2 = vld1q_u8((const uint8_t*)src + 32);
    uint8x16_t v3 = vld1q_u8((const uint8_t*)src + 48);
    vst1q_u8((uint8_t*)dst + 0, v0);
    vst1q_u8((uint8_t*)dst + 16, v1);
    vst1q_u8((uint8_t*)dst + 32, v2);
    vst1q_u8((uint8_t*)dst + 48, v3);
#endif
}

#if defined(OPT_AVX)
static INLINE void opt_cpy_256(void* dst, const void* src)
{
    __m256i m0 = _mm256_loadu_si256(((const __m256i*)src) + 0);
    __m256i m1 = _mm256_loadu_si256(((const __m256i*)src) + 1);
    __m256i m2 = _mm256_loadu_si256(((const __m256i*)src) + 2);
    __m256i m3 = _mm256_loadu_si256(((const __m256i*)src) + 3);
    __m256i m4 = _mm256_loadu_si256(((const __m256i*)src) + 4);
    __m256i m5 = _mm256_loadu_si256(((const __m256i*)src) + 5);
    __m256i m6 = _mm256_loadu_si256(((const __m256i*)src) + 6);
    __m256i m7 = _mm256_loadu_si256(((const __m256i*)src) + 7);
    _mm256_storeu_si256(((__m256i*)dst) + 0, m0);
    _mm256_storeu_si256(((__m256i*)dst) + 1, m1);
    _mm256_storeu_si256(((__m256i*)dst) + 2, m2);
    _mm256_storeu_si256(((__m256i*)dst) + 3, m3);
    _mm256_storeu_si256(((__m256i*)dst) + 4, m4);
    _mm256_storeu_si256(((__m256i*)dst) + 5, m5);
    _mm256_storeu_si256(((__m256i*)dst) + 6, m6);
    _mm256_storeu_si256(((__m256i*)dst) + 7, m7);
}
#endif


//---------------------------------------------------------------------
// tiny memory copy with jump table optimized
//---------------------------------------------------------------------
#if defined(OPT_AVX)
static INLINE void* opt_cpy_small(void* RESTRICT dst, const void* RESTRICT src, size_t size)
{
    unsigned char*       dd = ((unsigned char*)dst) + size;
    const unsigned char* ss = ((const unsigned char*)src) + size;

    switch (size) {
        case 128: opt_cpy_128(dd - 128, ss - 128);
        case 0: break;
        case 129: opt_cpy_128(dd - 129, ss - 129);
        case 1: dd[-1] = ss[-1]; break;
        case 130: opt_cpy_128(dd - 130, ss - 130);
        case 2: *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2)); break;
        case 131: opt_cpy_128(dd - 131, ss - 131);
        case 3:
            *((uint16_t*)(dd - 3)) = *((uint16_t*)(ss - 3));
            dd[-1]                 = ss[-1];
            break;
        case 132: opt_cpy_128(dd - 132, ss - 132);
        case 4: *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4)); break;
        case 133: opt_cpy_128(dd - 133, ss - 133);
        case 5:
            *((uint32_t*)(dd - 5)) = *((uint32_t*)(ss - 5));
            dd[-1]                 = ss[-1];
            break;
        case 134: opt_cpy_128(dd - 134, ss - 134);
        case 6:
            *((uint32_t*)(dd - 6)) = *((uint32_t*)(ss - 6));
            *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2));
            break;
        case 135: opt_cpy_128(dd - 135, ss - 135);
        case 7:
            *((uint32_t*)(dd - 7)) = *((uint32_t*)(ss - 7));
            *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
            break;
        case 136: opt_cpy_128(dd - 136, ss - 136);
        case 8: *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8)); break;
        case 137: opt_cpy_128(dd - 137, ss - 137);
        case 9:
            *((uint64_t*)(dd - 9)) = *((uint64_t*)(ss - 9));
            dd[-1]                 = ss[-1];
            break;
        case 138: opt_cpy_128(dd - 138, ss - 138);
        case 10:
            *((uint64_t*)(dd - 10)) = *((uint64_t*)(ss - 10));
            *((uint16_t*)(dd - 2))  = *((uint16_t*)(ss - 2));
            break;
        case 139: opt_cpy_128(dd - 139, ss - 139);
        case 11:
            *((uint64_t*)(dd - 11)) = *((uint64_t*)(ss - 11));
            *((uint32_t*)(dd - 4))  = *((uint32_t*)(ss - 4));
            break;
        case 140: opt_cpy_128(dd - 140, ss - 140);
        case 12:
            *((uint64_t*)(dd - 12)) = *((uint64_t*)(ss - 12));
            *((uint32_t*)(dd - 4))  = *((uint32_t*)(ss - 4));
            break;
        case 141: opt_cpy_128(dd - 141, ss - 141);
        case 13:
            *((uint64_t*)(dd - 13)) = *((uint64_t*)(ss - 13));
            *((uint64_t*)(dd - 8))  = *((uint64_t*)(ss - 8));
            break;
        case 142: opt_cpy_128(dd - 142, ss - 142);
        case 14:
            *((uint64_t*)(dd - 14)) = *((uint64_t*)(ss - 14));
            *((uint64_t*)(dd - 8))  = *((uint64_t*)(ss - 8));
            break;
        case 143: opt_cpy_128(dd - 143, ss - 143);
        case 15:
            *((uint64_t*)(dd - 15)) = *((uint64_t*)(ss - 15));
            *((uint64_t*)(dd - 8))  = *((uint64_t*)(ss - 8));
            break;
        case 144: opt_cpy_128(dd - 144, ss - 144);
        case 16: opt_cpy_16(dd - 16, ss - 16); break;
        case 145: opt_cpy_128(dd - 145, ss - 145);
        case 17:
            opt_cpy_16(dd - 17, ss - 17);
            dd[-1] = ss[-1];
            break;
        case 146: opt_cpy_128(dd - 146, ss - 146);
        case 18:
            opt_cpy_16(dd - 18, ss - 18);
            *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2));
            break;
        case 147: opt_cpy_128(dd - 147, ss - 147);
        case 19:
            opt_cpy_16(dd - 19, ss - 19);
            *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
            break;
        case 148: opt_cpy_128(dd - 148, ss - 148);
        case 20:
            opt_cpy_16(dd - 20, ss - 20);
            *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
            break;
        case 149: opt_cpy_128(dd - 149, ss - 149);
        case 21:
            opt_cpy_16(dd - 21, ss - 21);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 150: opt_cpy_128(dd - 150, ss - 150);
        case 22:
            opt_cpy_16(dd - 22, ss - 22);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 151: opt_cpy_128(dd - 151, ss - 151);
        case 23:
            opt_cpy_16(dd - 23, ss - 23);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 152: opt_cpy_128(dd - 152, ss - 152);
        case 24:
            opt_cpy_16(dd - 24, ss - 24);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 153: opt_cpy_128(dd - 153, ss - 153);
        case 25:
            opt_cpy_16(dd - 25, ss - 25);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 154: opt_cpy_128(dd - 154, ss - 154);
        case 26:
            opt_cpy_16(dd - 26, ss - 26);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 155: opt_cpy_128(dd - 155, ss - 155);
        case 27:
            opt_cpy_16(dd - 27, ss - 27);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 156: opt_cpy_128(dd - 156, ss - 156);
        case 28:
            opt_cpy_16(dd - 28, ss - 28);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 157: opt_cpy_128(dd - 157, ss - 157);
        case 29:
            opt_cpy_16(dd - 29, ss - 29);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 158: opt_cpy_128(dd - 158, ss - 158);
        case 30:
            opt_cpy_16(dd - 30, ss - 30);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 159: opt_cpy_128(dd - 159, ss - 159);
        case 31:
            opt_cpy_16(dd - 31, ss - 31);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 160: opt_cpy_128(dd - 160, ss - 160);
        case 32: opt_cpy_32(dd - 32, ss - 32); break;
        case 161: opt_cpy_128(dd - 161, ss - 161);
        case 33:
            opt_cpy_32(dd - 33, ss - 33);
            dd[-1] = ss[-1];
            break;
        case 162: opt_cpy_128(dd - 162, ss - 162);
        case 34:
            opt_cpy_32(dd - 34, ss - 34);
            *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2));
            break;
        case 163: opt_cpy_128(dd - 163, ss - 163);
        case 35:
            opt_cpy_32(dd - 35, ss - 35);
            *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
            break;
        case 164: opt_cpy_128(dd - 164, ss - 164);
        case 36:
            opt_cpy_32(dd - 36, ss - 36);
            *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
            break;
        case 165: opt_cpy_128(dd - 165, ss - 165);
        case 37:
            opt_cpy_32(dd - 37, ss - 37);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 166: opt_cpy_128(dd - 166, ss - 166);
        case 38:
            opt_cpy_32(dd - 38, ss - 38);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 167: opt_cpy_128(dd - 167, ss - 167);
        case 39:
            opt_cpy_32(dd - 39, ss - 39);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 168: opt_cpy_128(dd - 168, ss - 168);
        case 40:
            opt_cpy_32(dd - 40, ss - 40);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 169: opt_cpy_128(dd - 169, ss - 169);
        case 41:
            opt_cpy_32(dd - 41, ss - 41);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 170: opt_cpy_128(dd - 170, ss - 170);
        case 42:
            opt_cpy_32(dd - 42, ss - 42);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 171: opt_cpy_128(dd - 171, ss - 171);
        case 43:
            opt_cpy_32(dd - 43, ss - 43);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 172: opt_cpy_128(dd - 172, ss - 172);
        case 44:
            opt_cpy_32(dd - 44, ss - 44);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 173: opt_cpy_128(dd - 173, ss - 173);
        case 45:
            opt_cpy_32(dd - 45, ss - 45);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 174: opt_cpy_128(dd - 174, ss - 174);
        case 46:
            opt_cpy_32(dd - 46, ss - 46);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 175: opt_cpy_128(dd - 175, ss - 175);
        case 47:
            opt_cpy_32(dd - 47, ss - 47);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 176: opt_cpy_128(dd - 176, ss - 176);
        case 48:
            opt_cpy_32(dd - 48, ss - 48);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 177: opt_cpy_128(dd - 177, ss - 177);
        case 49:
            opt_cpy_32(dd - 49, ss - 49);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 178: opt_cpy_128(dd - 178, ss - 178);
        case 50:
            opt_cpy_32(dd - 50, ss - 50);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 179: opt_cpy_128(dd - 179, ss - 179);
        case 51:
            opt_cpy_32(dd - 51, ss - 51);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 180: opt_cpy_128(dd - 180, ss - 180);
        case 52:
            opt_cpy_32(dd - 52, ss - 52);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 181: opt_cpy_128(dd - 181, ss - 181);
        case 53:
            opt_cpy_32(dd - 53, ss - 53);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 182: opt_cpy_128(dd - 182, ss - 182);
        case 54:
            opt_cpy_32(dd - 54, ss - 54);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 183: opt_cpy_128(dd - 183, ss - 183);
        case 55:
            opt_cpy_32(dd - 55, ss - 55);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 184: opt_cpy_128(dd - 184, ss - 184);
        case 56:
            opt_cpy_32(dd - 56, ss - 56);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 185: opt_cpy_128(dd - 185, ss - 185);
        case 57:
            opt_cpy_32(dd - 57, ss - 57);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 186: opt_cpy_128(dd - 186, ss - 186);
        case 58:
            opt_cpy_32(dd - 58, ss - 58);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 187: opt_cpy_128(dd - 187, ss - 187);
        case 59:
            opt_cpy_32(dd - 59, ss - 59);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 188: opt_cpy_128(dd - 188, ss - 188);
        case 60:
            opt_cpy_32(dd - 60, ss - 60);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 189: opt_cpy_128(dd - 189, ss - 189);
        case 61:
            opt_cpy_32(dd - 61, ss - 61);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 190: opt_cpy_128(dd - 190, ss - 190);
        case 62:
            opt_cpy_32(dd - 62, ss - 62);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 191: opt_cpy_128(dd - 191, ss - 191);
        case 63:
            opt_cpy_32(dd - 63, ss - 63);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 192: opt_cpy_128(dd - 192, ss - 192);
        case 64: opt_cpy_64(dd - 64, ss - 64); break;
        case 193: opt_cpy_128(dd - 193, ss - 193);
        case 65:
            opt_cpy_64(dd - 65, ss - 65);
            dd[-1] = ss[-1];
            break;
        case 194: opt_cpy_128(dd - 194, ss - 194);
        case 66:
            opt_cpy_64(dd - 66, ss - 66);
            *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2));
            break;
        case 195: opt_cpy_128(dd - 195, ss - 195);
        case 67:
            opt_cpy_64(dd - 67, ss - 67);
            *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
            break;
        case 196: opt_cpy_128(dd - 196, ss - 196);
        case 68:
            opt_cpy_64(dd - 68, ss - 68);
            *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
            break;
        case 197: opt_cpy_128(dd - 197, ss - 197);
        case 69:
            opt_cpy_64(dd - 69, ss - 69);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 198: opt_cpy_128(dd - 198, ss - 198);
        case 70:
            opt_cpy_64(dd - 70, ss - 70);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 199: opt_cpy_128(dd - 199, ss - 199);
        case 71:
            opt_cpy_64(dd - 71, ss - 71);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 200: opt_cpy_128(dd - 200, ss - 200);
        case 72:
            opt_cpy_64(dd - 72, ss - 72);
            *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8));
            break;
        case 201: opt_cpy_128(dd - 201, ss - 201);
        case 73:
            opt_cpy_64(dd - 73, ss - 73);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 202: opt_cpy_128(dd - 202, ss - 202);
        case 74:
            opt_cpy_64(dd - 74, ss - 74);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 203: opt_cpy_128(dd - 203, ss - 203);
        case 75:
            opt_cpy_64(dd - 75, ss - 75);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 204: opt_cpy_128(dd - 204, ss - 204);
        case 76:
            opt_cpy_64(dd - 76, ss - 76);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 205: opt_cpy_128(dd - 205, ss - 205);
        case 77:
            opt_cpy_64(dd - 77, ss - 77);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 206: opt_cpy_128(dd - 206, ss - 206);
        case 78:
            opt_cpy_64(dd - 78, ss - 78);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 207: opt_cpy_128(dd - 207, ss - 207);
        case 79:
            opt_cpy_64(dd - 79, ss - 79);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 208: opt_cpy_128(dd - 208, ss - 208);
        case 80:
            opt_cpy_64(dd - 80, ss - 80);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 209: opt_cpy_128(dd - 209, ss - 209);
        case 81:
            opt_cpy_64(dd - 81, ss - 81);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 210: opt_cpy_128(dd - 210, ss - 210);
        case 82:
            opt_cpy_64(dd - 82, ss - 82);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 211: opt_cpy_128(dd - 211, ss - 211);
        case 83:
            opt_cpy_64(dd - 83, ss - 83);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 212: opt_cpy_128(dd - 212, ss - 212);
        case 84:
            opt_cpy_64(dd - 84, ss - 84);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 213: opt_cpy_128(dd - 213, ss - 213);
        case 85:
            opt_cpy_64(dd - 85, ss - 85);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 214: opt_cpy_128(dd - 214, ss - 214);
        case 86:
            opt_cpy_64(dd - 86, ss - 86);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 215: opt_cpy_128(dd - 215, ss - 215);
        case 87:
            opt_cpy_64(dd - 87, ss - 87);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 216: opt_cpy_128(dd - 216, ss - 216);
        case 88:
            opt_cpy_64(dd - 88, ss - 88);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 217: opt_cpy_128(dd - 217, ss - 217);
        case 89:
            opt_cpy_64(dd - 89, ss - 89);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 218: opt_cpy_128(dd - 218, ss - 218);
        case 90:
            opt_cpy_64(dd - 90, ss - 90);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 219: opt_cpy_128(dd - 219, ss - 219);
        case 91:
            opt_cpy_64(dd - 91, ss - 91);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 220: opt_cpy_128(dd - 220, ss - 220);
        case 92:
            opt_cpy_64(dd - 92, ss - 92);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 221: opt_cpy_128(dd - 221, ss - 221);
        case 93:
            opt_cpy_64(dd - 93, ss - 93);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 222: opt_cpy_128(dd - 222, ss - 222);
        case 94:
            opt_cpy_64(dd - 94, ss - 94);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 223: opt_cpy_128(dd - 223, ss - 223);
        case 95:
            opt_cpy_64(dd - 95, ss - 95);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 224: opt_cpy_128(dd - 224, ss - 224);
        case 96:
            opt_cpy_64(dd - 96, ss - 96);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 225: opt_cpy_128(dd - 225, ss - 225);
        case 97:
            opt_cpy_64(dd - 97, ss - 97);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 226: opt_cpy_128(dd - 226, ss - 226);
        case 98:
            opt_cpy_64(dd - 98, ss - 98);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 227: opt_cpy_128(dd - 227, ss - 227);
        case 99:
            opt_cpy_64(dd - 99, ss - 99);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 228: opt_cpy_128(dd - 228, ss - 228);
        case 100:
            opt_cpy_64(dd - 100, ss - 100);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 229: opt_cpy_128(dd - 229, ss - 229);
        case 101:
            opt_cpy_64(dd - 101, ss - 101);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 230: opt_cpy_128(dd - 230, ss - 230);
        case 102:
            opt_cpy_64(dd - 102, ss - 102);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 231: opt_cpy_128(dd - 231, ss - 231);
        case 103:
            opt_cpy_64(dd - 103, ss - 103);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 232: opt_cpy_128(dd - 232, ss - 232);
        case 104:
            opt_cpy_64(dd - 104, ss - 104);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 233: opt_cpy_128(dd - 233, ss - 233);
        case 105:
            opt_cpy_64(dd - 105, ss - 105);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 234: opt_cpy_128(dd - 234, ss - 234);
        case 106:
            opt_cpy_64(dd - 106, ss - 106);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 235: opt_cpy_128(dd - 235, ss - 235);
        case 107:
            opt_cpy_64(dd - 107, ss - 107);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 236: opt_cpy_128(dd - 236, ss - 236);
        case 108:
            opt_cpy_64(dd - 108, ss - 108);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 237: opt_cpy_128(dd - 237, ss - 237);
        case 109:
            opt_cpy_64(dd - 109, ss - 109);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 238: opt_cpy_128(dd - 238, ss - 238);
        case 110:
            opt_cpy_64(dd - 110, ss - 110);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 239: opt_cpy_128(dd - 239, ss - 239);
        case 111:
            opt_cpy_64(dd - 111, ss - 111);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 240: opt_cpy_128(dd - 240, ss - 240);
        case 112:
            opt_cpy_64(dd - 112, ss - 112);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 241: opt_cpy_128(dd - 241, ss - 241);
        case 113:
            opt_cpy_64(dd - 113, ss - 113);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 242: opt_cpy_128(dd - 242, ss - 242);
        case 114:
            opt_cpy_64(dd - 114, ss - 114);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 243: opt_cpy_128(dd - 243, ss - 243);
        case 115:
            opt_cpy_64(dd - 115, ss - 115);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 244: opt_cpy_128(dd - 244, ss - 244);
        case 116:
            opt_cpy_64(dd - 116, ss - 116);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 245: opt_cpy_128(dd - 245, ss - 245);
        case 117:
            opt_cpy_64(dd - 117, ss - 117);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 246: opt_cpy_128(dd - 246, ss - 246);
        case 118:
            opt_cpy_64(dd - 118, ss - 118);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 247: opt_cpy_128(dd - 247, ss - 247);
        case 119:
            opt_cpy_64(dd - 119, ss - 119);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 248: opt_cpy_128(dd - 248, ss - 248);
        case 120:
            opt_cpy_64(dd - 120, ss - 120);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 249: opt_cpy_128(dd - 249, ss - 249);
        case 121:
            opt_cpy_64(dd - 121, ss - 121);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 250: opt_cpy_128(dd - 250, ss - 250);
        case 122:
            opt_cpy_64(dd - 122, ss - 122);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 251: opt_cpy_128(dd - 251, ss - 251);
        case 123:
            opt_cpy_64(dd - 123, ss - 123);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 252: opt_cpy_128(dd - 252, ss - 252);
        case 124:
            opt_cpy_64(dd - 124, ss - 124);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 253: opt_cpy_128(dd - 253, ss - 253);
        case 125:
            opt_cpy_64(dd - 125, ss - 125);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 254: opt_cpy_128(dd - 254, ss - 254);
        case 126:
            opt_cpy_64(dd - 126, ss - 126);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 255: opt_cpy_128(dd - 255, ss - 255);
        case 127:
            opt_cpy_64(dd - 127, ss - 127);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 256: opt_cpy_256(dd - 256, ss - 256); break;
    }

    return dst;
}
#elif defined(OPT_NEON)
static INLINE void opt_cpy_small(void* RESTRICT dst, const void* RESTRICT src, std::size_t size)
{
    unsigned char*       dd = ((unsigned char*)dst) + size;
    const unsigned char* ss = ((const unsigned char*)src) + size;

    switch (size) {
        case 0: break;
        case 1: dd[-1] = ss[-1]; break;
        case 2: *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2)); break;
        case 3:
            *((uint16_t*)(dd - 3)) = *((uint16_t*)(ss - 3));
            dd[-1]                 = ss[-1];
            break;
        case 4: *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4)); break;
        case 5:
            *((uint32_t*)(dd - 5)) = *((uint32_t*)(ss - 5));
            dd[-1]                 = ss[-1];
            break;
        case 6:
            *((uint32_t*)(dd - 6)) = *((uint32_t*)(ss - 6));
            *((uint16_t*)(dd - 2)) = *((uint16_t*)(ss - 2));
            break;
        case 7:
            *((uint32_t*)(dd - 7)) = *((uint32_t*)(ss - 7));
            *((uint32_t*)(dd - 4)) = *((uint32_t*)(ss - 4));
            break;
        case 8: *((uint64_t*)(dd - 8)) = *((uint64_t*)(ss - 8)); break;
        case 9:
            *((uint64_t*)(dd - 9)) = *((uint64_t*)(ss - 9));
            dd[-1]                 = ss[-1];
            break;
        case 10:
            *((uint64_t*)(dd - 10)) = *((uint64_t*)(ss - 10));
            *((uint16_t*)(dd - 2))  = *((uint16_t*)(ss - 2));
            break;
        case 11:
            *((uint64_t*)(dd - 11)) = *((uint64_t*)(ss - 11));
            *((uint32_t*)(dd - 4))  = *((uint32_t*)(ss - 4));
            break;
        case 12:
            *((uint64_t*)(dd - 12)) = *((uint64_t*)(ss - 12));
            *((uint32_t*)(dd - 4))  = *((uint32_t*)(ss - 4));
            break;
        case 13:
            *((uint64_t*)(dd - 13)) = *((uint64_t*)(ss - 13));
            *((uint64_t*)(dd - 8))  = *((uint64_t*)(ss - 8));
            break;
        case 14:
            *((uint64_t*)(dd - 14)) = *((uint64_t*)(ss - 14));
            *((uint64_t*)(dd - 8))  = *((uint64_t*)(ss - 8));
            break;
        case 15:
            *((uint64_t*)(dd - 15)) = *((uint64_t*)(ss - 15));
            *((uint64_t*)(dd - 8))  = *((uint64_t*)(ss - 8));
            break;
        case 16: opt_cpy_16(dd - 16, ss - 16); break;
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
            opt_cpy_16(dd - size, ss - size);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 32: opt_cpy_32(dd - 32, ss - 32); break;
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
        case 50:
        case 51:
        case 52:
        case 53:
        case 54:
        case 55:
        case 56:
        case 57:
        case 58:
        case 59:
        case 60:
        case 61:
        case 62:
        case 63:
            opt_cpy_32(dd - size, ss - size);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 64: opt_cpy_64(dd - 64, ss - 64); break;
        case 65:
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
        case 76:
        case 77:
        case 78:
        case 79:
        case 80:
        case 81:
        case 82:
        case 83:
        case 84:
        case 85:
        case 86:
        case 87:
        case 88:
        case 89:
        case 90:
        case 91:
        case 92:
        case 93:
        case 94:
        case 95:
        case 96:
        case 97:
        case 98:
        case 99:
        case 100:
        case 101:
        case 102:
        case 103:
        case 104:
        case 105:
        case 106:
        case 107:
        case 108:
        case 109:
        case 110:
        case 111:
        case 112:
        case 113:
        case 114:
        case 115:
        case 116:
        case 117:
        case 118:
        case 119:
        case 120:
        case 121:
        case 122:
        case 123:
        case 124:
        case 125:
        case 126:
        case 127:
            opt_cpy_64(dd - size, ss - size);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 128: opt_cpy_128(dd - 128, ss - 128); break;
    }
}
#endif

inline void opt_cpy(void* RESTRICT dstVoid, const void* RESTRICT srcVoid, std::size_t size)
{
    if (dstVoid == nullptr || srcVoid == nullptr || size == 0) {
        return;
    }

    const char* RESTRICT src       = reinterpret_cast<const char*>(srcVoid);
    char* RESTRICT       dst       = reinterpret_cast<char*>(dstVoid);
    std::size_t          remaining = size;


#ifdef OPT_AVX
    if (remaining < 256) {
        opt_cpy_small(dst, src, remaining);
        return;
    }
    constexpr std::size_t kChunkBytes             = 256;
    constexpr std::size_t kAvxWidth               = 32;
    constexpr std::size_t kMediumPrefetchMinBytes = 8192;
    constexpr std::size_t kMediumPrefetchMaxBytes = 64 * 1024;
    constexpr std::size_t kMediumPrefetchDistance = 256;

    const bool useMediumPrefetch =
        remaining - kMediumPrefetchMinBytes <= kMediumPrefetchMaxBytes - kMediumPrefetchMinBytes;

    __m256i c0, c1, c2, c3, c4, c5, c6, c7;
    for (; remaining >= kChunkBytes; remaining -= kChunkBytes) {
        if (useMediumPrefetch && remaining > kMediumPrefetchDistance) {
            _mm_prefetch(src + kMediumPrefetchDistance + 0, _MM_HINT_T0);
            _mm_prefetch(src + kMediumPrefetchDistance + 64, _MM_HINT_T0);
            _mm_prefetch(src + kMediumPrefetchDistance + 128, _MM_HINT_T0);
            _mm_prefetch(src + kMediumPrefetchDistance + 192, _MM_HINT_T0);
        }

        c0 = _mm256_load_si256(reinterpret_cast<const __m256i*>(src) + 0);
        c1 = _mm256_load_si256(reinterpret_cast<const __m256i*>(src) + 1);
        c2 = _mm256_load_si256(reinterpret_cast<const __m256i*>(src) + 2);
        c3 = _mm256_load_si256(reinterpret_cast<const __m256i*>(src) + 3);
        c4 = _mm256_load_si256(reinterpret_cast<const __m256i*>(src) + 4);
        c5 = _mm256_load_si256(reinterpret_cast<const __m256i*>(src) + 5);
        c6 = _mm256_load_si256(reinterpret_cast<const __m256i*>(src) + 6);
        c7 = _mm256_load_si256(reinterpret_cast<const __m256i*>(src) + 7);

        _mm256_store_si256(reinterpret_cast<__m256i*>(dst) + 0, c0);
        _mm256_store_si256(reinterpret_cast<__m256i*>(dst) + 1, c1);
        _mm256_store_si256(reinterpret_cast<__m256i*>(dst) + 2, c2);
        _mm256_store_si256(reinterpret_cast<__m256i*>(dst) + 3, c3);
        _mm256_store_si256(reinterpret_cast<__m256i*>(dst) + 4, c4);
        _mm256_store_si256(reinterpret_cast<__m256i*>(dst) + 5, c5);
        _mm256_store_si256(reinterpret_cast<__m256i*>(dst) + 6, c6);
        _mm256_store_si256(reinterpret_cast<__m256i*>(dst) + 7, c7);

        src += kChunkBytes;
        dst += kChunkBytes;
    }

    if (remaining > 0) {
        opt_cpy_small(dst, src, remaining);
    }

#elif defined(OPT_NEON)
    if (remaining < 128) {
        opt_cpy_small(dst, src, remaining);
        return;
    }
    constexpr std::size_t kNeonWidth = 16;
    for (; remaining >= kNeonWidth; remaining -= kNeonWidth) {
        const uint8x16_t data = vld1q_u8(reinterpret_cast<const std::uint8_t*>(src));
        vst1q_u8(reinterpret_cast<std::uint8_t*>(dst), data);
        src += kNeonWidth;
        dst += kNeonWidth;
    }
#endif

    for (; remaining != 0; --remaining) {
        *dst++ = *src++;
    }
}