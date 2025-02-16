/* -*- c++ -*- */
/*
 * Copyright 2012, 2013, 2014 Free Software Foundation, Inc.
 *
 * This file is part of VOLK
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/*!
 * \page volk_32fc_x2_dot_prod_32fc
 *
 * \b Overview
 *
 * This block computes the dot product (or inner product) between two
 * vectors, the \p input and \p taps vectors. Given a set of \p
 * num_points taps, the result is the sum of products between the two
 * vectors. The result is a single value stored in the \p result
 * address and is returned as a complex float.
 *
 * <b>Dispatcher Prototype</b>
 * \code
 * void volk_32fc_x2_dot_prod_32fc(lv_32fc_t* result, const lv_32fc_t* input, const
 * lv_32fc_t* taps, unsigned int num_points) \endcode
 *
 * \b Inputs
 * \li input: vector of complex floats.
 * \li taps:  complex float taps.
 * \li num_points: number of samples in both \p input and \p taps.
 *
 * \b Outputs
 * \li result: pointer to a complex float value to hold the dot product result.
 *
 * \b Example
 * \code
 * int N = 10000;
 *
 * <FIXME>
 *
 * volk_32fc_x2_dot_prod_32fc();
 *
 * \endcode
 */

#ifndef INCLUDED_volk_32fc_x2_dot_prod_32fc_u_H
#define INCLUDED_volk_32fc_x2_dot_prod_32fc_u_H

#include <stdio.h>
#include <string.h>
#include <volk/volk_common.h>
#include <volk/volk_complex.h>


#ifdef LV_HAVE_RISCV64
extern void volk_32fc_x2_dot_prod_32fc_sifive_u74(lv_32fc_t* result,
                                                  const lv_32fc_t* input,
                                                  const lv_32fc_t* taps,
                                                  unsigned int num_points);
#endif

#ifdef LV_HAVE_GENERIC


static inline void volk_32fc_x2_dot_prod_32fc_generic(lv_32fc_t* result,
                                                      const lv_32fc_t* input,
                                                      const lv_32fc_t* taps,
                                                      unsigned int num_points)
{

    float* res = (float*)result;
    float* in = (float*)input;
    float* tp = (float*)taps;
    unsigned int n_2_ccomplex_blocks = num_points / 2;

    float sum0[2] = { 0, 0 };
    float sum1[2] = { 0, 0 };
    unsigned int i = 0;

    for (i = 0; i < n_2_ccomplex_blocks; ++i) {
        sum0[0] += in[0] * tp[0] - in[1] * tp[1];
        sum0[1] += in[0] * tp[1] + in[1] * tp[0];
        sum1[0] += in[2] * tp[2] - in[3] * tp[3];
        sum1[1] += in[2] * tp[3] + in[3] * tp[2];

        in += 4;
        tp += 4;
    }

    res[0] = sum0[0] + sum1[0];
    res[1] = sum0[1] + sum1[1];

    // Cleanup if we had an odd number of points
    if (num_points & 1) {
        *result += input[num_points - 1] * taps[num_points - 1];
    }
}

#endif /*LV_HAVE_GENERIC*/


#ifdef LV_HAVE_SSE3

#include <pmmintrin.h>

static inline void volk_32fc_x2_dot_prod_32fc_u_sse3(lv_32fc_t* result,
                                                     const lv_32fc_t* input,
                                                     const lv_32fc_t* taps,
                                                     unsigned int num_points)
{

    lv_32fc_t dotProduct;
    memset(&dotProduct, 0x0, 2 * sizeof(float));

    unsigned int number = 0;
    const unsigned int halfPoints = num_points / 2;
    unsigned int isodd = num_points & 1;

    __m128 x, y, yl, yh, z, tmp1, tmp2, dotProdVal;

    const lv_32fc_t* a = input;
    const lv_32fc_t* b = taps;

    dotProdVal = _mm_setzero_ps();

    for (; number < halfPoints; number++) {

        x = _mm_loadu_ps((float*)a); // Load the ar + ai, br + bi as ar,ai,br,bi
        y = _mm_loadu_ps((float*)b); // Load the cr + ci, dr + di as cr,ci,dr,di

        yl = _mm_moveldup_ps(y); // Load yl with cr,cr,dr,dr
        yh = _mm_movehdup_ps(y); // Load yh with ci,ci,di,di

        tmp1 = _mm_mul_ps(x, yl); // tmp1 = ar*cr,ai*cr,br*dr,bi*dr

        x = _mm_shuffle_ps(x, x, 0xB1); // Re-arrange x to be ai,ar,bi,br

        tmp2 = _mm_mul_ps(x, yh); // tmp2 = ai*ci,ar*ci,bi*di,br*di

        z = _mm_addsub_ps(tmp1,
                          tmp2); // ar*cr-ai*ci, ai*cr+ar*ci, br*dr-bi*di, bi*dr+br*di

        dotProdVal =
            _mm_add_ps(dotProdVal, z); // Add the complex multiplication results together

        a += 2;
        b += 2;
    }

    __VOLK_ATTR_ALIGNED(16) lv_32fc_t dotProductVector[2];

    _mm_storeu_ps((float*)dotProductVector,
                  dotProdVal); // Store the results back into the dot product vector

    dotProduct += (dotProductVector[0] + dotProductVector[1]);

    if (isodd) {
        dotProduct += input[num_points - 1] * taps[num_points - 1];
    }

    *result = dotProduct;
}

#endif /*LV_HAVE_SSE3*/

#ifdef LV_HAVE_AVX

#include <immintrin.h>

static inline void volk_32fc_x2_dot_prod_32fc_u_avx(lv_32fc_t* result,
                                                    const lv_32fc_t* input,
                                                    const lv_32fc_t* taps,
                                                    unsigned int num_points)
{

    unsigned int isodd = num_points & 3;
    unsigned int i = 0;
    lv_32fc_t dotProduct;
    memset(&dotProduct, 0x0, 2 * sizeof(float));

    unsigned int number = 0;
    const unsigned int quarterPoints = num_points / 4;

    __m256 x, y, yl, yh, z, tmp1, tmp2, dotProdVal;

    const lv_32fc_t* a = input;
    const lv_32fc_t* b = taps;

    dotProdVal = _mm256_setzero_ps();

    for (; number < quarterPoints; number++) {
        x = _mm256_loadu_ps((float*)a); // Load a,b,e,f as ar,ai,br,bi,er,ei,fr,fi
        y = _mm256_loadu_ps((float*)b); // Load c,d,g,h as cr,ci,dr,di,gr,gi,hr,hi

        yl = _mm256_moveldup_ps(y); // Load yl with cr,cr,dr,dr,gr,gr,hr,hr
        yh = _mm256_movehdup_ps(y); // Load yh with ci,ci,di,di,gi,gi,hi,hi

        tmp1 = _mm256_mul_ps(x, yl); // tmp1 = ar*cr,ai*cr,br*dr,bi*dr ...

        x = _mm256_shuffle_ps(x, x, 0xB1); // Re-arrange x to be ai,ar,bi,br,ei,er,fi,fr

        tmp2 = _mm256_mul_ps(x, yh); // tmp2 = ai*ci,ar*ci,bi*di,br*di ...

        z = _mm256_addsub_ps(tmp1,
                             tmp2); // ar*cr-ai*ci, ai*cr+ar*ci, br*dr-bi*di, bi*dr+br*di

        dotProdVal = _mm256_add_ps(dotProdVal,
                                   z); // Add the complex multiplication results together

        a += 4;
        b += 4;
    }

    __VOLK_ATTR_ALIGNED(32) lv_32fc_t dotProductVector[4];

    _mm256_storeu_ps((float*)dotProductVector,
                     dotProdVal); // Store the results back into the dot product vector

    dotProduct += (dotProductVector[0] + dotProductVector[1] + dotProductVector[2] +
                   dotProductVector[3]);

    for (i = num_points - isodd; i < num_points; i++) {
        dotProduct += input[i] * taps[i];
    }

    *result = dotProduct;
}

#endif /*LV_HAVE_AVX*/

#if LV_HAVE_AVX && LV_HAVE_FMA
#include <immintrin.h>

static inline void volk_32fc_x2_dot_prod_32fc_u_avx_fma(lv_32fc_t* result,
                                                        const lv_32fc_t* input,
                                                        const lv_32fc_t* taps,
                                                        unsigned int num_points)
{

    unsigned int isodd = num_points & 3;
    unsigned int i = 0;
    lv_32fc_t dotProduct;
    memset(&dotProduct, 0x0, 2 * sizeof(float));

    unsigned int number = 0;
    const unsigned int quarterPoints = num_points / 4;

    __m256 x, y, yl, yh, z, tmp1, tmp2, dotProdVal;

    const lv_32fc_t* a = input;
    const lv_32fc_t* b = taps;

    dotProdVal = _mm256_setzero_ps();

    for (; number < quarterPoints; number++) {

        x = _mm256_loadu_ps((float*)a); // Load a,b,e,f as ar,ai,br,bi,er,ei,fr,fi
        y = _mm256_loadu_ps((float*)b); // Load c,d,g,h as cr,ci,dr,di,gr,gi,hr,hi

        yl = _mm256_moveldup_ps(y); // Load yl with cr,cr,dr,dr,gr,gr,hr,hr
        yh = _mm256_movehdup_ps(y); // Load yh with ci,ci,di,di,gi,gi,hi,hi

        tmp1 = x;

        x = _mm256_shuffle_ps(x, x, 0xB1); // Re-arrange x to be ai,ar,bi,br,ei,er,fi,fr

        tmp2 = _mm256_mul_ps(x, yh); // tmp2 = ai*ci,ar*ci,bi*di,br*di ...

        z = _mm256_fmaddsub_ps(
            tmp1, yl, tmp2); // ar*cr-ai*ci, ai*cr+ar*ci, br*dr-bi*di, bi*dr+br*di

        dotProdVal = _mm256_add_ps(dotProdVal,
                                   z); // Add the complex multiplication results together

        a += 4;
        b += 4;
    }

    __VOLK_ATTR_ALIGNED(32) lv_32fc_t dotProductVector[4];

    _mm256_storeu_ps((float*)dotProductVector,
                     dotProdVal); // Store the results back into the dot product vector

    dotProduct += (dotProductVector[0] + dotProductVector[1] + dotProductVector[2] +
                   dotProductVector[3]);

    for (i = num_points - isodd; i < num_points; i++) {
        dotProduct += input[i] * taps[i];
    }

    *result = dotProduct;
}

#endif /*LV_HAVE_AVX && LV_HAVE_FMA*/

#endif /*INCLUDED_volk_32fc_x2_dot_prod_32fc_u_H*/

#ifndef INCLUDED_volk_32fc_x2_dot_prod_32fc_a_H
#define INCLUDED_volk_32fc_x2_dot_prod_32fc_a_H

#include <stdio.h>
#include <string.h>
#include <volk/volk_common.h>
#include <volk/volk_complex.h>


#ifdef LV_HAVE_SSE3

#include <pmmintrin.h>

static inline void volk_32fc_x2_dot_prod_32fc_a_sse3(lv_32fc_t* result,
                                                     const lv_32fc_t* input,
                                                     const lv_32fc_t* taps,
                                                     unsigned int num_points)
{

    const unsigned int num_bytes = num_points * 8;
    unsigned int isodd = num_points & 1;

    lv_32fc_t dotProduct;
    memset(&dotProduct, 0x0, 2 * sizeof(float));

    unsigned int number = 0;
    const unsigned int halfPoints = num_bytes >> 4;

    __m128 x, y, yl, yh, z, tmp1, tmp2, dotProdVal;

    const lv_32fc_t* a = input;
    const lv_32fc_t* b = taps;

    dotProdVal = _mm_setzero_ps();

    for (; number < halfPoints; number++) {

        x = _mm_load_ps((float*)a); // Load the ar + ai, br + bi as ar,ai,br,bi
        y = _mm_load_ps((float*)b); // Load the cr + ci, dr + di as cr,ci,dr,di

        yl = _mm_moveldup_ps(y); // Load yl with cr,cr,dr,dr
        yh = _mm_movehdup_ps(y); // Load yh with ci,ci,di,di

        tmp1 = _mm_mul_ps(x, yl); // tmp1 = ar*cr,ai*cr,br*dr,bi*dr

        x = _mm_shuffle_ps(x, x, 0xB1); // Re-arrange x to be ai,ar,bi,br

        tmp2 = _mm_mul_ps(x, yh); // tmp2 = ai*ci,ar*ci,bi*di,br*di

        z = _mm_addsub_ps(tmp1,
                          tmp2); // ar*cr-ai*ci, ai*cr+ar*ci, br*dr-bi*di, bi*dr+br*di

        dotProdVal =
            _mm_add_ps(dotProdVal, z); // Add the complex multiplication results together

        a += 2;
        b += 2;
    }

    __VOLK_ATTR_ALIGNED(16) lv_32fc_t dotProductVector[2];

    _mm_store_ps((float*)dotProductVector,
                 dotProdVal); // Store the results back into the dot product vector

    dotProduct += (dotProductVector[0] + dotProductVector[1]);

    if (isodd) {
        dotProduct += input[num_points - 1] * taps[num_points - 1];
    }

    *result = dotProduct;
}

#endif /*LV_HAVE_SSE3*/


#ifdef LV_HAVE_NEON
#include <arm_neon.h>

static inline void volk_32fc_x2_dot_prod_32fc_neon(lv_32fc_t* result,
                                                   const lv_32fc_t* input,
                                                   const lv_32fc_t* taps,
                                                   unsigned int num_points)
{

    unsigned int quarter_points = num_points / 4;
    unsigned int number;

    lv_32fc_t* a_ptr = (lv_32fc_t*)taps;
    lv_32fc_t* b_ptr = (lv_32fc_t*)input;
    // for 2-lane vectors, 1st lane holds the real part,
    // 2nd lane holds the imaginary part
    float32x4x2_t a_val, b_val, c_val, accumulator;
    float32x4x2_t tmp_real, tmp_imag;
    accumulator.val[0] = vdupq_n_f32(0);
    accumulator.val[1] = vdupq_n_f32(0);

    for (number = 0; number < quarter_points; ++number) {
        a_val = vld2q_f32((float*)a_ptr); // a0r|a1r|a2r|a3r || a0i|a1i|a2i|a3i
        b_val = vld2q_f32((float*)b_ptr); // b0r|b1r|b2r|b3r || b0i|b1i|b2i|b3i
        __VOLK_PREFETCH(a_ptr + 8);
        __VOLK_PREFETCH(b_ptr + 8);

        // multiply the real*real and imag*imag to get real result
        // a0r*b0r|a1r*b1r|a2r*b2r|a3r*b3r
        tmp_real.val[0] = vmulq_f32(a_val.val[0], b_val.val[0]);
        // a0i*b0i|a1i*b1i|a2i*b2i|a3i*b3i
        tmp_real.val[1] = vmulq_f32(a_val.val[1], b_val.val[1]);

        // Multiply cross terms to get the imaginary result
        // a0r*b0i|a1r*b1i|a2r*b2i|a3r*b3i
        tmp_imag.val[0] = vmulq_f32(a_val.val[0], b_val.val[1]);
        // a0i*b0r|a1i*b1r|a2i*b2r|a3i*b3r
        tmp_imag.val[1] = vmulq_f32(a_val.val[1], b_val.val[0]);

        c_val.val[0] = vsubq_f32(tmp_real.val[0], tmp_real.val[1]);
        c_val.val[1] = vaddq_f32(tmp_imag.val[0], tmp_imag.val[1]);

        accumulator.val[0] = vaddq_f32(accumulator.val[0], c_val.val[0]);
        accumulator.val[1] = vaddq_f32(accumulator.val[1], c_val.val[1]);

        a_ptr += 4;
        b_ptr += 4;
    }
    lv_32fc_t accum_result[4];
    vst2q_f32((float*)accum_result, accumulator);
    *result = accum_result[0] + accum_result[1] + accum_result[2] + accum_result[3];

    // tail case
    for (number = quarter_points * 4; number < num_points; ++number) {
        *result += (*a_ptr++) * (*b_ptr++);
    }
}
#endif /*LV_HAVE_NEON*/

#ifdef LV_HAVE_NEON
#include <arm_neon.h>
static inline void volk_32fc_x2_dot_prod_32fc_neon_opttests(lv_32fc_t* result,
                                                            const lv_32fc_t* input,
                                                            const lv_32fc_t* taps,
                                                            unsigned int num_points)
{

    unsigned int quarter_points = num_points / 4;
    unsigned int number;

    lv_32fc_t* a_ptr = (lv_32fc_t*)taps;
    lv_32fc_t* b_ptr = (lv_32fc_t*)input;
    // for 2-lane vectors, 1st lane holds the real part,
    // 2nd lane holds the imaginary part
    float32x4x2_t a_val, b_val, accumulator;
    float32x4x2_t tmp_imag;
    accumulator.val[0] = vdupq_n_f32(0);
    accumulator.val[1] = vdupq_n_f32(0);

    for (number = 0; number < quarter_points; ++number) {
        a_val = vld2q_f32((float*)a_ptr); // a0r|a1r|a2r|a3r || a0i|a1i|a2i|a3i
        b_val = vld2q_f32((float*)b_ptr); // b0r|b1r|b2r|b3r || b0i|b1i|b2i|b3i
        __VOLK_PREFETCH(a_ptr + 8);
        __VOLK_PREFETCH(b_ptr + 8);

        // do the first multiply
        tmp_imag.val[1] = vmulq_f32(a_val.val[1], b_val.val[0]);
        tmp_imag.val[0] = vmulq_f32(a_val.val[0], b_val.val[0]);

        // use multiply accumulate/subtract to get result
        tmp_imag.val[1] = vmlaq_f32(tmp_imag.val[1], a_val.val[0], b_val.val[1]);
        tmp_imag.val[0] = vmlsq_f32(tmp_imag.val[0], a_val.val[1], b_val.val[1]);

        accumulator.val[0] = vaddq_f32(accumulator.val[0], tmp_imag.val[0]);
        accumulator.val[1] = vaddq_f32(accumulator.val[1], tmp_imag.val[1]);

        // increment pointers
        a_ptr += 4;
        b_ptr += 4;
    }
    lv_32fc_t accum_result[4];
    vst2q_f32((float*)accum_result, accumulator);
    *result = accum_result[0] + accum_result[1] + accum_result[2] + accum_result[3];

    // tail case
    for (number = quarter_points * 4; number < num_points; ++number) {
        *result += (*a_ptr++) * (*b_ptr++);
    }
}
#endif /*LV_HAVE_NEON*/

#ifdef LV_HAVE_NEON
static inline void volk_32fc_x2_dot_prod_32fc_neon_optfma(lv_32fc_t* result,
                                                          const lv_32fc_t* input,
                                                          const lv_32fc_t* taps,
                                                          unsigned int num_points)
{

    unsigned int quarter_points = num_points / 4;
    unsigned int number;

    lv_32fc_t* a_ptr = (lv_32fc_t*)taps;
    lv_32fc_t* b_ptr = (lv_32fc_t*)input;
    // for 2-lane vectors, 1st lane holds the real part,
    // 2nd lane holds the imaginary part
    float32x4x2_t a_val, b_val, accumulator1, accumulator2;
    accumulator1.val[0] = vdupq_n_f32(0);
    accumulator1.val[1] = vdupq_n_f32(0);
    accumulator2.val[0] = vdupq_n_f32(0);
    accumulator2.val[1] = vdupq_n_f32(0);

    for (number = 0; number < quarter_points; ++number) {
        a_val = vld2q_f32((float*)a_ptr); // a0r|a1r|a2r|a3r || a0i|a1i|a2i|a3i
        b_val = vld2q_f32((float*)b_ptr); // b0r|b1r|b2r|b3r || b0i|b1i|b2i|b3i
        __VOLK_PREFETCH(a_ptr + 8);
        __VOLK_PREFETCH(b_ptr + 8);

        // use 2 accumulators to remove inter-instruction data dependencies
        accumulator1.val[0] = vmlaq_f32(accumulator1.val[0], a_val.val[0], b_val.val[0]);
        accumulator1.val[1] = vmlaq_f32(accumulator1.val[1], a_val.val[0], b_val.val[1]);
        accumulator2.val[0] = vmlsq_f32(accumulator2.val[0], a_val.val[1], b_val.val[1]);
        accumulator2.val[1] = vmlaq_f32(accumulator2.val[1], a_val.val[1], b_val.val[0]);
        // increment pointers
        a_ptr += 4;
        b_ptr += 4;
    }
    accumulator1.val[0] = vaddq_f32(accumulator1.val[0], accumulator2.val[0]);
    accumulator1.val[1] = vaddq_f32(accumulator1.val[1], accumulator2.val[1]);
    lv_32fc_t accum_result[4];
    vst2q_f32((float*)accum_result, accumulator1);
    *result = accum_result[0] + accum_result[1] + accum_result[2] + accum_result[3];

    // tail case
    for (number = quarter_points * 4; number < num_points; ++number) {
        *result += (*a_ptr++) * (*b_ptr++);
    }
}
#endif /*LV_HAVE_NEON*/

#ifdef LV_HAVE_NEON
static inline void volk_32fc_x2_dot_prod_32fc_neon_optfmaunroll(lv_32fc_t* result,
                                                                const lv_32fc_t* input,
                                                                const lv_32fc_t* taps,
                                                                unsigned int num_points)
{
    // NOTE: GCC does a poor job with this kernel, but the equivalent ASM code is very
    // fast

    unsigned int quarter_points = num_points / 8;
    unsigned int number;

    lv_32fc_t* a_ptr = (lv_32fc_t*)taps;
    lv_32fc_t* b_ptr = (lv_32fc_t*)input;
    // for 2-lane vectors, 1st lane holds the real part,
    // 2nd lane holds the imaginary part
    float32x4x4_t a_val, b_val, accumulator1, accumulator2;
    float32x4x2_t reduced_accumulator;
    accumulator1.val[0] = vdupq_n_f32(0);
    accumulator1.val[1] = vdupq_n_f32(0);
    accumulator1.val[2] = vdupq_n_f32(0);
    accumulator1.val[3] = vdupq_n_f32(0);
    accumulator2.val[0] = vdupq_n_f32(0);
    accumulator2.val[1] = vdupq_n_f32(0);
    accumulator2.val[2] = vdupq_n_f32(0);
    accumulator2.val[3] = vdupq_n_f32(0);

    // 8 input regs, 8 accumulators -> 16/16 neon regs are used
    for (number = 0; number < quarter_points; ++number) {
        a_val = vld4q_f32((float*)a_ptr); // a0r|a1r|a2r|a3r || a0i|a1i|a2i|a3i
        b_val = vld4q_f32((float*)b_ptr); // b0r|b1r|b2r|b3r || b0i|b1i|b2i|b3i
        __VOLK_PREFETCH(a_ptr + 8);
        __VOLK_PREFETCH(b_ptr + 8);

        // use 2 accumulators to remove inter-instruction data dependencies
        accumulator1.val[0] = vmlaq_f32(accumulator1.val[0], a_val.val[0], b_val.val[0]);
        accumulator1.val[1] = vmlaq_f32(accumulator1.val[1], a_val.val[0], b_val.val[1]);

        accumulator1.val[2] = vmlaq_f32(accumulator1.val[2], a_val.val[2], b_val.val[2]);
        accumulator1.val[3] = vmlaq_f32(accumulator1.val[3], a_val.val[2], b_val.val[3]);

        accumulator2.val[0] = vmlsq_f32(accumulator2.val[0], a_val.val[1], b_val.val[1]);
        accumulator2.val[1] = vmlaq_f32(accumulator2.val[1], a_val.val[1], b_val.val[0]);

        accumulator2.val[2] = vmlsq_f32(accumulator2.val[2], a_val.val[3], b_val.val[3]);
        accumulator2.val[3] = vmlaq_f32(accumulator2.val[3], a_val.val[3], b_val.val[2]);
        // increment pointers
        a_ptr += 8;
        b_ptr += 8;
    }
    // reduce 8 accumulator lanes down to 2 (1 real and 1 imag)
    accumulator1.val[0] = vaddq_f32(accumulator1.val[0], accumulator1.val[2]);
    accumulator1.val[1] = vaddq_f32(accumulator1.val[1], accumulator1.val[3]);
    accumulator2.val[0] = vaddq_f32(accumulator2.val[0], accumulator2.val[2]);
    accumulator2.val[1] = vaddq_f32(accumulator2.val[1], accumulator2.val[3]);
    reduced_accumulator.val[0] = vaddq_f32(accumulator1.val[0], accumulator2.val[0]);
    reduced_accumulator.val[1] = vaddq_f32(accumulator1.val[1], accumulator2.val[1]);
    // now reduce accumulators to scalars
    lv_32fc_t accum_result[4];
    vst2q_f32((float*)accum_result, reduced_accumulator);
    *result = accum_result[0] + accum_result[1] + accum_result[2] + accum_result[3];

    // tail case
    for (number = quarter_points * 8; number < num_points; ++number) {
        *result += (*a_ptr++) * (*b_ptr++);
    }
}
#endif /*LV_HAVE_NEON*/


#ifdef LV_HAVE_AVX

#include <immintrin.h>

static inline void volk_32fc_x2_dot_prod_32fc_a_avx(lv_32fc_t* result,
                                                    const lv_32fc_t* input,
                                                    const lv_32fc_t* taps,
                                                    unsigned int num_points)
{

    unsigned int isodd = num_points & 3;
    unsigned int i = 0;
    lv_32fc_t dotProduct;
    memset(&dotProduct, 0x0, 2 * sizeof(float));

    unsigned int number = 0;
    const unsigned int quarterPoints = num_points / 4;

    __m256 x, y, yl, yh, z, tmp1, tmp2, dotProdVal;

    const lv_32fc_t* a = input;
    const lv_32fc_t* b = taps;

    dotProdVal = _mm256_setzero_ps();

    for (; number < quarterPoints; number++) {

        x = _mm256_load_ps((float*)a); // Load a,b,e,f as ar,ai,br,bi,er,ei,fr,fi
        y = _mm256_load_ps((float*)b); // Load c,d,g,h as cr,ci,dr,di,gr,gi,hr,hi

        yl = _mm256_moveldup_ps(y); // Load yl with cr,cr,dr,dr,gr,gr,hr,hr
        yh = _mm256_movehdup_ps(y); // Load yh with ci,ci,di,di,gi,gi,hi,hi

        tmp1 = _mm256_mul_ps(x, yl); // tmp1 = ar*cr,ai*cr,br*dr,bi*dr ...

        x = _mm256_shuffle_ps(x, x, 0xB1); // Re-arrange x to be ai,ar,bi,br,ei,er,fi,fr

        tmp2 = _mm256_mul_ps(x, yh); // tmp2 = ai*ci,ar*ci,bi*di,br*di ...

        z = _mm256_addsub_ps(tmp1,
                             tmp2); // ar*cr-ai*ci, ai*cr+ar*ci, br*dr-bi*di, bi*dr+br*di

        dotProdVal = _mm256_add_ps(dotProdVal,
                                   z); // Add the complex multiplication results together

        a += 4;
        b += 4;
    }

    __VOLK_ATTR_ALIGNED(32) lv_32fc_t dotProductVector[4];

    _mm256_store_ps((float*)dotProductVector,
                    dotProdVal); // Store the results back into the dot product vector

    dotProduct += (dotProductVector[0] + dotProductVector[1] + dotProductVector[2] +
                   dotProductVector[3]);

    for (i = num_points - isodd; i < num_points; i++) {
        dotProduct += input[i] * taps[i];
    }

    *result = dotProduct;
}

#endif /*LV_HAVE_AVX*/

#if LV_HAVE_AVX && LV_HAVE_FMA
#include <immintrin.h>

static inline void volk_32fc_x2_dot_prod_32fc_a_avx_fma(lv_32fc_t* result,
                                                        const lv_32fc_t* input,
                                                        const lv_32fc_t* taps,
                                                        unsigned int num_points)
{

    unsigned int isodd = num_points & 3;
    unsigned int i = 0;
    lv_32fc_t dotProduct;
    memset(&dotProduct, 0x0, 2 * sizeof(float));

    unsigned int number = 0;
    const unsigned int quarterPoints = num_points / 4;

    __m256 x, y, yl, yh, z, tmp1, tmp2, dotProdVal;

    const lv_32fc_t* a = input;
    const lv_32fc_t* b = taps;

    dotProdVal = _mm256_setzero_ps();

    for (; number < quarterPoints; number++) {

        x = _mm256_load_ps((float*)a); // Load a,b,e,f as ar,ai,br,bi,er,ei,fr,fi
        y = _mm256_load_ps((float*)b); // Load c,d,g,h as cr,ci,dr,di,gr,gi,hr,hi

        yl = _mm256_moveldup_ps(y); // Load yl with cr,cr,dr,dr,gr,gr,hr,hr
        yh = _mm256_movehdup_ps(y); // Load yh with ci,ci,di,di,gi,gi,hi,hi

        tmp1 = x;

        x = _mm256_shuffle_ps(x, x, 0xB1); // Re-arrange x to be ai,ar,bi,br,ei,er,fi,fr

        tmp2 = _mm256_mul_ps(x, yh); // tmp2 = ai*ci,ar*ci,bi*di,br*di ...

        z = _mm256_fmaddsub_ps(
            tmp1, yl, tmp2); // ar*cr-ai*ci, ai*cr+ar*ci, br*dr-bi*di, bi*dr+br*di

        dotProdVal = _mm256_add_ps(dotProdVal,
                                   z); // Add the complex multiplication results together

        a += 4;
        b += 4;
    }

    __VOLK_ATTR_ALIGNED(32) lv_32fc_t dotProductVector[4];

    _mm256_store_ps((float*)dotProductVector,
                    dotProdVal); // Store the results back into the dot product vector

    dotProduct += (dotProductVector[0] + dotProductVector[1] + dotProductVector[2] +
                   dotProductVector[3]);

    for (i = num_points - isodd; i < num_points; i++) {
        dotProduct += input[i] * taps[i];
    }

    *result = dotProduct;
}

#endif /*LV_HAVE_AVX && LV_HAVE_FMA*/


#endif /*INCLUDED_volk_32fc_x2_dot_prod_32fc_a_H*/
