/*
 * Reference (portable) implementations of the ops.
 * Compile with -std=c99
 */

#include "ops_abi.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdio.h>

static ops_table_t g_ops_table;

static ops_status_t generic_mul_f32(const float *a, const float *b, float *out, size_t n);
static ops_status_t generic_div_f32(const float *a, const float *b, float *out, size_t n);
static ops_status_t generic_log_f32(const float *in, float *out, size_t n);
static ops_status_t generic_biquad_f32(const float *in, float *out, size_t n,
                                       const float b[3], const float a[3], float state[2]);
static ops_status_t generic_fft_f32(float *real, float *imag, size_t n, int inverse);

static int is_power_of_two(size_t n) { return n && ((n & (n - 1)) == 0); }

static ops_status_t generic_mul_f32(const float *a, const float *b, float *out, size_t n) {
    if (!a || !b || !out) return OPS_ERR_INVALID_ARG;
    for (size_t i = 0; i < n; ++i) out[i] = a[i] * b[i];
    return OPS_OK;
}

static ops_status_t generic_div_f32(const float *a, const float *b, float *out, size_t n) {
    if (!a || !b || !out) return OPS_ERR_INVALID_ARG;
    for (size_t i = 0; i < n; ++i) out[i] = a[i] / b[i];
    return OPS_OK;
}

static ops_status_t generic_log_f32(const float *in, float *out, size_t n) {
    if (!in || !out) return OPS_ERR_INVALID_ARG;
    for (size_t i = 0; i < n; ++i) out[i] = logf(in[i]);
    return OPS_OK;
}

static ops_status_t generic_biquad_f32(const float *in, float *out, size_t n,
                                       const float b[3], const float a[3], float state[2]) {
    if (!in || !out || !b || !a || !state) return OPS_ERR_INVALID_ARG;
    float d1 = state[0];
    float d2 = state[1];
    float b0 = b[0], b1 = b[1], b2 = b[2];
    float a1 = a[1], a2 = a[2];
    for (size_t i = 0; i < n; ++i) {
        float x = in[i];
        float y = b0 * x + d1;
        out[i] = y;
        float nd1 = b1 * x - a1 * y + d2;
        float nd2 = b2 * x - a2 * y;
        d1 = nd1; d2 = nd2;
    }
    state[0] = d1; state[1] = d2;
    return OPS_OK;
}

static void bit_reverse_permute(float *real, float *imag, size_t n) {
    size_t j = 0;
    for (size_t i = 1; i < n; ++i) {
        size_t k = n >> 1;
        while (j & k) { j ^= k; k >>= 1; }
        j |= k;
        if (i < j) {
            float tr = real[i]; real[i] = real[j]; real[j] = tr;
            float ti = imag[i]; imag[i] = imag[j]; imag[j] = ti;
        }
    }
}

static ops_status_t generic_fft_f32(float *real, float *imag, size_t n, int inverse) {
    if (!real || !imag) return OPS_ERR_INVALID_ARG;
    if (!is_power_of_two(n)) return OPS_ERR_INVALID_ARG;
    bit_reverse_permute(real, imag, n);
    for (size_t step = 2; step <= n; step <<= 1) {
        size_t half = step >> 1;
        double theta = (inverse ? 2.0 : -2.0) * 3.14159265358979323846 / (double)step;
        float wpr = (float)(cos(theta) - 1.0);
        float wpi = (float)sin(theta);
        for (size_t m = 0; m < n; m += step) {
            float wr = 1.0f, wi = 0.0f;
            for (size_t k = 0; k < half; ++k) {
                size_t i = m + k;
                size_t j = i + half;
                float tr = wr * real[j] - wi * imag[j];
                float ti = wr * imag[j] + wi * real[j];
                real[j] = real[i] - tr; imag[j] = imag[i] - ti;
                real[i] = real[i] + tr; imag[i] = imag[i] + ti;
                float tmp = wr;
                wr = wr + (tmp * wpr - wi * wpi);
                wi = wi + (wi * wpr + tmp * wpi);
            }
        }
    }
    if (inverse) {
        float invn = 1.0f / (float)n;
        for (size_t i = 0; i < n; ++i) { real[i] *= invn; imag[i] *= invn; }
    }
    return OPS_OK;
}

static void setup_generic_table(void) {
    g_ops_table.abi_version = OPS_ABI_VERSION;
    g_ops_table.mul_f32 = generic_mul_f32;
    g_ops_table.div_f32 = generic_div_f32;
    g_ops_table.log_f32 = generic_log_f32;
    g_ops_table.biquad_f32 = generic_biquad_f32;
    g_ops_table.fft_f32 = generic_fft_f32;
}

ops_status_t ops_library_init(void) { setup_generic_table(); return OPS_OK; }

ops_status_t ops_register_backend(const ops_table_t *backend) {
    if (!backend) return OPS_ERR_INVALID_ARG;
    if (backend->abi_version != OPS_ABI_VERSION) return OPS_ERR_UNSUPPORTED;
    if (backend->mul_f32) g_ops_table.mul_f32 = backend->mul_f32;
    if (backend->div_f32) g_ops_table.div_f32 = backend->div_f32;
    if (backend->log_f32) g_ops_table.log_f32 = backend->log_f32;
    if (backend->biquad_f32) g_ops_table.biquad_f32 = backend->biquad_f32;
    if (backend->fft_f32) g_ops_table.fft_f32 = backend->fft_f32;
    return OPS_OK;
}

const ops_table_t *ops_get_table(void) { if (g_ops_table.abi_version != OPS_ABI_VERSION) ops_library_init(); return &g_ops_table; }
