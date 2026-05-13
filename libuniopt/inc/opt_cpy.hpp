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
            opt_cpy_16(dst, src);
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
            opt_cpy_32(dst, src);
            opt_cpy_16(dd - 16, ss - 16);
            break;
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
            opt_cpy_32(dst, src);
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
            opt_cpy_64(dst, src);
            opt_cpy_16(dd - 16, ss - 16);
            break;
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
            opt_cpy_64(dst, src);
            opt_cpy_32(dd - 32, ss - 32);
            break;
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
            opt_cpy_64(dst, src);
            opt_cpy_32(dd - 48, ss - 48);
            opt_cpy_16(dd - 16, ss - 16);
            break;
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
            opt_cpy_64(dst, src);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 128: opt_cpy_128(dd - 128, ss - 128); break;
        case 129:
        case 130:
        case 131:
        case 132:
        case 133:
        case 134:
        case 135:
        case 136:
        case 137:
        case 138:
        case 139:
        case 140:
        case 141:
        case 142:
        case 143:
        case 144:
            opt_cpy_128(dst, src);
            opt_cpy_16(dd - 16, ss - 16);
            break;
        case 145:
        case 146:
        case 147:
        case 148:
        case 149:
        case 150:
        case 151:
        case 152:
        case 153:
        case 154:
        case 155:
        case 156:
        case 157:
        case 158:
        case 159:
        case 160:
            opt_cpy_128(dst, src);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 161:
        case 162:
        case 163:
        case 164:
        case 165:
        case 166:
        case 167:
        case 168:
        case 169:
        case 170:
        case 171:
        case 172:
        case 173:
        case 174:
        case 175:
        case 176:
        case 177:
        case 178:
        case 179:
        case 180:
        case 181:
        case 182:
        case 183:
        case 184:
        case 185:
        case 186:
        case 187:
        case 188:
        case 189:
        case 190:
        case 191:
        case 192:
            opt_cpy_128(dst, src);
            opt_cpy_64(dd - 64, ss - 64);
            break;
        case 193:
        case 194:
        case 195:
        case 196:
        case 197:
        case 198:
        case 199:
        case 200:
        case 201:
        case 202:
        case 203:
        case 204:
        case 205:
        case 206:
        case 207:
        case 208:
        case 209:
        case 210:
        case 211:
        case 212:
        case 213:
        case 214:
        case 215:
        case 216:
        case 217:
        case 218:
        case 219:
        case 220:
        case 221:
        case 222:
        case 223:
        case 224:
            opt_cpy_128(dst, src);
            opt_cpy_64(dd - 96, ss - 96);
            opt_cpy_32(dd - 32, ss - 32);
            break;
        case 225:
        case 226:
        case 227:
        case 228:
        case 229:
        case 230:
        case 231:
        case 232:
        case 233:
        case 234:
        case 235:
        case 236:
        case 237:
        case 238:
        case 239:
        case 240:
        case 241:
        case 242:
        case 243:
        case 244:
        case 245:
        case 246:
        case 247:
        case 248:
        case 249:
        case 250:
        case 251:
        case 252:
        case 253:
        case 254:
        case 255:
            opt_cpy_128(dst, src);
            opt_cpy_128(dd - 128, ss - 128);
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
            opt_cpy_32(dst, src);
            opt_cpy_16(dd - 16, ss - 16);
            break;
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
            opt_cpy_32(dst, src);
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
            opt_cpy_64(dst, src);
            opt_cpy_16(dd - 16, ss - 16);
            break;
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
            opt_cpy_64(dst, src);
            opt_cpy_32(dd - 32, ss - 32);
            break;
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
            opt_cpy_64(dst, src);
            opt_cpy_32(dd - 48, ss - 48); // 中间 32 字节
            opt_cpy_16(dd - 16, ss - 16); // 末尾 16 字节
            break;
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
            opt_cpy_64(dst, src);
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

    constexpr std::size_t kMediumPrefetchMinBytes = 8192;
    constexpr std::size_t kMediumPrefetchMaxBytes = 64 * 1024;
    constexpr std::size_t kMediumPrefetchDistance = 256;
    // 假设 32MB 为 L3 缓存阈值，实际应用中建议通过 CPUID 动态获取
    constexpr std::size_t kHugeThreshold = 32 * 1024 * 1024;

    if (remaining < kHugeThreshold) {
        constexpr std::size_t kMediumPrefetchMinBytes = 8192;
        constexpr std::size_t kMediumPrefetchMaxBytes = 64 * 1024;
        constexpr std::size_t kMediumPrefetchDistance = 256;

        const bool useMediumPrefetch =
            remaining - kMediumPrefetchMinBytes <= kMediumPrefetchMaxBytes - kMediumPrefetchMinBytes;

        __m256i c0, c1, c2, c3, c4, c5, c6, c7;
        if (useMediumPrefetch) {
            for (; remaining >= 256; remaining -= 256) {
                _mm_prefetch(src + kMediumPrefetchDistance + 0, _MM_HINT_T0);
                _mm_prefetch(src + kMediumPrefetchDistance + 64, _MM_HINT_T0);
                _mm_prefetch(src + kMediumPrefetchDistance + 128, _MM_HINT_T0);
                _mm_prefetch(src + kMediumPrefetchDistance + 192, _MM_HINT_T0);

                c0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 0);
                c1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 1);
                c2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 2);
                c3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 3);
                c4 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 4);
                c5 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 5);
                c6 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 6);
                c7 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 7);

                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 0, c0);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 1, c1);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 2, c2);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 3, c3);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 4, c4);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 5, c5);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 6, c6);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 7, c7);

                src += 256;
                dst += 256;
            }
        } else {
            for (; remaining >= 256; remaining -= 256) {
                c0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 0);
                c1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 1);
                c2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 2);
                c3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 3);
                c4 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 4);
                c5 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 5);
                c6 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 6);
                c7 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 7);

                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 0, c0);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 1, c1);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 2, c2);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 3, c3);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 4, c4);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 5, c5);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 6, c6);
                _mm256_storeu_si256(reinterpret_cast<__m256i*>(dst) + 7, c7);

                src += 256;
                dst += 256;
            }
        }
    } else {
        // 超大内存路径: 使用 Non-Temporal Stores (Streaming)
        __m256i c0, c1, c2, c3, c4, c5, c6, c7;
        for (; remaining >= 256; remaining -= 256) {
            // NTA 预取：提示数据不保留在缓存中
            _mm_prefetch(src + 512, _MM_HINT_NTA);

            c0 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 0);
            c1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 1);
            c2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 2);
            c3 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 3);
            c4 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 4);
            c5 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 5);
            c6 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 6);
            c7 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(src) + 7);

            // 绕过缓存直接写入内存
            _mm256_stream_si256(reinterpret_cast<__m256i*>(dst) + 0, c0);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(dst) + 1, c1);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(dst) + 2, c2);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(dst) + 3, c3);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(dst) + 4, c4);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(dst) + 5, c5);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(dst) + 6, c6);
            _mm256_stream_si256(reinterpret_cast<__m256i*>(dst) + 7, c7);

            src += 256;
            dst += 256;
        }
        _mm_sfence(); // 确保异步流写入完成
    }

    if (remaining > 0) {
        opt_cpy_small(dst, src, remaining);
    }

#elif defined(OPT_NEON)
    if (remaining < 128) {
        opt_cpy_small(dst, src, remaining);
        return;
    }

    constexpr std::size_t kHugeThreshold = 32 * 1024 * 1024;

    if (remaining < kHugeThreshold) {
        while (remaining >= 128) {
            uint8x16_t v0 = vld1q_u8(reinterpret_cast<const uint8_t*>(src) + 0);
            uint8x16_t v1 = vld1q_u8(reinterpret_cast<const uint8_t*>(src) + 16);
            uint8x16_t v2 = vld1q_u8(reinterpret_cast<const uint8_t*>(src) + 32);
            uint8x16_t v3 = vld1q_u8(reinterpret_cast<const uint8_t*>(src) + 48);
            uint8x16_t v4 = vld1q_u8(reinterpret_cast<const uint8_t*>(src) + 64);
            uint8x16_t v5 = vld1q_u8(reinterpret_cast<const uint8_t*>(src) + 80);
            uint8x16_t v6 = vld1q_u8(reinterpret_cast<const uint8_t*>(src) + 96);
            uint8x16_t v7 = vld1q_u8(reinterpret_cast<const uint8_t*>(src) + 112);

            vst1q_u8(reinterpret_cast<uint8_t*>(dst) + 0, v0);
            vst1q_u8(reinterpret_cast<uint8_t*>(dst) + 16, v1);
            vst1q_u8(reinterpret_cast<uint8_t*>(dst) + 32, v2);
            vst1q_u8(reinterpret_cast<uint8_t*>(dst) + 48, v3);
            vst1q_u8(reinterpret_cast<uint8_t*>(dst) + 64, v4);
            vst1q_u8(reinterpret_cast<uint8_t*>(dst) + 80, v5);
            vst1q_u8(reinterpret_cast<uint8_t*>(dst) + 96, v6);
            vst1q_u8(reinterpret_cast<uint8_t*>(dst) + 112, v7);

            src += 128;
            dst += 128;
            remaining -= 128;
        }
    } else {
        // NEON 超大内存路径: 使用 Non-Temporal hint
        while (remaining >= 128) {
            // 使用内联汇编调用 STNP (Store Non-temporal Pair)
            // 每次处理 32 字节 (两个 16 字节寄存器)
            __asm__ __volatile__(
                "prfm pldl1strm, [%0, #256] \n"
                "ld1 {v0.16b, v1.16b, v2.16b, v3.16b}, [%0], #64 \n"
                "ld1 {v4.16b, v5.16b, v6.16b, v7.16b}, [%0], #64 \n"
                "stnp q0, q1, [%1] \n"
                "stnp q2, q3, [%1, #32] \n"
                "stnp q4, q5, [%1, #64] \n"
                "stnp q6, q7, [%1, #96] \n"
                : "+r"(src)
                : "r"(dst)
                : "v0", "v1", "v2", "v3", "v4", "v5", "v6", "v7", "memory"
            );
            dst += 128;
            remaining -= 128;
        }
    }

    if (remaining > 0) {
        opt_cpy_small(dst, src, remaining);
    }
#endif

    for (; remaining != 0; --remaining) {
        *dst++ = *src++;
    }
}