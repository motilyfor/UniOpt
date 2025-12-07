#ifndef OPS_ABI_H
#define OPS_ABI_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ABI version: increment if the ops_table_t layout changes */
#define OPS_ABI_VERSION 1

/* Common return codes */
typedef enum {
    OPS_OK = 0,
    OPS_ERR_UNSUPPORTED = -1,
    OPS_ERR_INVALID_ARG = -2,
} ops_status_t;

/* Function pointer typedefs for supported operators. */

typedef ops_status_t (*ops_mul_f32_fn)(
    const float *a, const float *b, float *out, size_t n);

typedef ops_status_t (*ops_div_f32_fn)(
    const float *a, const float *b, float *out, size_t n);

typedef ops_status_t (*ops_log_f32_fn)(
    const float *in, float *out, size_t n);

typedef ops_status_t (*ops_biquad_f32_fn)(
    const float *in, float *out, size_t n,
    const float b[3], const float a[3], float state[2]);

typedef ops_status_t (*ops_fft_f32_fn)(
    float *real, float *imag, size_t n, int inverse);

typedef struct {
    uint32_t abi_version; /* must be OPS_ABI_VERSION */
    ops_mul_f32_fn mul_f32;
    ops_div_f32_fn div_f32;
    ops_log_f32_fn log_f32;
    ops_biquad_f32_fn biquad_f32;
    ops_fft_f32_fn fft_f32;
} ops_table_t;

ops_status_t ops_library_init(void);
ops_status_t ops_register_backend(const ops_table_t *backend);
const ops_table_t *ops_get_table(void);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* OPS_ABI_H */
