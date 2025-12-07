#include <stdio.h>
#include "ops_abi.h"

static void print_array(const char *name, const float *a, size_t n) {
    printf("%s: [", name);
    for (size_t i = 0; i < n; ++i) printf("%g%s", a[i], i + 1 == n ? "" : ", ");
    printf("]\n");
}

int main(void) {
    if (ops_library_init() != OPS_OK) {
        fprintf(stderr, "Failed to init ops library\n");
        return 1;
    }
    const ops_table_t *ops = ops_get_table();
    size_t n = 8;
    float a[8] = {1,2,3,4,5,6,7,8};
    float b[8] = {2,2,2,2,2,2,2,2};
    float out[8];
    ops->mul_f32(a, b, out, n);
    print_array("mul", out, n);
    ops->div_f32(a, b, out, n);
    print_array("div", out, n);
    ops->log_f32(a, out, n);
    print_array("log", out, n);
    float bq_b[3] = {0.5f, 0.0f, 0.0f};
    float bq_a[3] = {1.0f, 0.0f, 0.0f};
    float state[2] = {0.0f, 0.0f};
    ops->biquad_f32(a, out, n, bq_b, bq_a, state);
    print_array("biquad (gain 0.5)", out, n);
    float real[8]; float imag[8];
    for (size_t i = 0; i < n; ++i) { real[i] = (float)i; imag[i] = 0.0f; }
    ops->fft_f32(real, imag, n, 0);
    printf("FFT real: "); for (size_t i = 0; i < n; ++i) printf("%g ", real[i]); printf("\n");
    ops->fft_f32(real, imag, n, 1);
    printf("IFFT real (reconstructed): "); for (size_t i = 0; i < n; ++i) printf("%g ", real[i]); printf("\n");
    return 0;
}
