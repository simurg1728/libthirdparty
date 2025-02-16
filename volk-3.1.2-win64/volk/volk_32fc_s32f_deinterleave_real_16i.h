/* -*- c++ -*- */
/*
 * Copyright 2012, 2014 Free Software Foundation, Inc.
 *
 * This file is part of VOLK
 *
 * SPDX-License-Identifier: LGPL-3.0-or-later
 */

/*!
 * \page volk_32fc_s32f_deinterleave_real_16i
 *
 * \b Overview
 *
 * Deinterleaves the complex floating point vector and return the real
 * part (inphase) of the samples scaled to 16-bit shorts.
 *
 * <b>Dispatcher Prototype</b>
 * \code
 * void volk_32fc_s32f_deinterleave_real_16i(int16_t* iBuffer, const lv_32fc_t*
 * complexVector, const float scalar, unsigned int num_points) \endcode
 *
 * \b Inputs
 * \li complexVector: The complex input vector.
 * \li scalar: The value to be multiplied against each of the input vectors..
 * \li num_points: The number of complex data values to be deinterleaved.
 *
 * \b Outputs
 * \li iBuffer: The I buffer output data.
 *
 * \b Example
 * Generate points around the unit circle and map them to integers with
 * magnitude 50 to preserve smallest deltas.
 * \code
 *   int N = 10;
 *   unsigned int alignment = volk_get_alignment();
 *   lv_32fc_t* in  = (lv_32fc_t*)volk_malloc(sizeof(lv_32fc_t)*N, alignment);
 *   int16_t* out = (int16_t*)volk_malloc(sizeof(int16_t)*N, alignment);
 *   float scale = 50.f;
 *
 *   for(unsigned int ii = 0; ii < N/2; ++ii){
 *       // Generate points around the unit circle
 *       float real = -4.f * ((float)ii / (float)N) + 1.f;
 *       float imag = std::sqrt(1.f - real * real);
 *       in[ii] = lv_cmake(real, imag);
 *       in[ii+N/2] = lv_cmake(-real, -imag);
 *   }
 *
 *   volk_32fc_s32f_deinterleave_real_16i(out, in, scale, N);
 *
 *   for(unsigned int ii = 0; ii < N; ++ii){
 *       printf("out[%u] = %i\n", ii, out[ii]);
 *   }
 *
 *   volk_free(in);
 *   volk_free(out);
 * \endcode
 */

#ifndef INCLUDED_volk_32fc_s32f_deinterleave_real_16i_a_H
#define INCLUDED_volk_32fc_s32f_deinterleave_real_16i_a_H

#include <inttypes.h>
#include <stdio.h>
#include <volk/volk_common.h>


#ifdef LV_HAVE_AVX2
#include <immintrin.h>

static inline void
volk_32fc_s32f_deinterleave_real_16i_a_avx2(int16_t* iBuffer,
                                            const lv_32fc_t* complexVector,
                                            const float scalar,
                                            unsigned int num_points)
{
    unsigned int number = 0;
    const unsigned int eighthPoints = num_points / 8;

    const float* complexVectorPtr = (float*)complexVector;
    int16_t* iBufferPtr = iBuffer;

    __m256 vScalar = _mm256_set1_ps(scalar);

    __m256 cplxValue1, cplxValue2, iValue;
    __m256i a;
    __m128i b;

    __m256i idx = _mm256_set_epi32(3, 3, 3, 3, 5, 1, 4, 0);

    for (; number < eighthPoints; number++) {
        cplxValue1 = _mm256_load_ps(complexVectorPtr);
        complexVectorPtr += 8;

        cplxValue2 = _mm256_load_ps(complexVectorPtr);
        complexVectorPtr += 8;

        // Arrange in i1i2i3i4 format
        iValue = _mm256_shuffle_ps(cplxValue1, cplxValue2, _MM_SHUFFLE(2, 0, 2, 0));

        iValue = _mm256_mul_ps(iValue, vScalar);

        a = _mm256_cvtps_epi32(iValue);
        a = _mm256_packs_epi32(a, a);
        a = _mm256_permutevar8x32_epi32(a, idx);
        b = _mm256_extracti128_si256(a, 0);

        _mm_store_si128((__m128i*)iBufferPtr, b);
        iBufferPtr += 8;
    }

    number = eighthPoints * 8;
    iBufferPtr = &iBuffer[number];
    for (; number < num_points; number++) {
        *iBufferPtr++ = (int16_t)rintf(*complexVectorPtr++ * scalar);
        complexVectorPtr++;
    }
}


#endif /* LV_HAVE_AVX2 */

#ifdef LV_HAVE_SSE
#include <xmmintrin.h>

static inline void
volk_32fc_s32f_deinterleave_real_16i_a_sse(int16_t* iBuffer,
                                           const lv_32fc_t* complexVector,
                                           const float scalar,
                                           unsigned int num_points)
{
    unsigned int number = 0;
    const unsigned int quarterPoints = num_points / 4;

    const float* complexVectorPtr = (float*)complexVector;
    int16_t* iBufferPtr = iBuffer;

    __m128 vScalar = _mm_set_ps1(scalar);

    __m128 cplxValue1, cplxValue2, iValue;

    __VOLK_ATTR_ALIGNED(16) float floatBuffer[4];

    for (; number < quarterPoints; number++) {
        cplxValue1 = _mm_load_ps(complexVectorPtr);
        complexVectorPtr += 4;

        cplxValue2 = _mm_load_ps(complexVectorPtr);
        complexVectorPtr += 4;

        // Arrange in i1i2i3i4 format
        iValue = _mm_shuffle_ps(cplxValue1, cplxValue2, _MM_SHUFFLE(2, 0, 2, 0));

        iValue = _mm_mul_ps(iValue, vScalar);

        _mm_store_ps(floatBuffer, iValue);
        *iBufferPtr++ = (int16_t)rintf(floatBuffer[0]);
        *iBufferPtr++ = (int16_t)rintf(floatBuffer[1]);
        *iBufferPtr++ = (int16_t)rintf(floatBuffer[2]);
        *iBufferPtr++ = (int16_t)rintf(floatBuffer[3]);
    }

    number = quarterPoints * 4;
    iBufferPtr = &iBuffer[number];
    for (; number < num_points; number++) {
        *iBufferPtr++ = (int16_t)rintf(*complexVectorPtr++ * scalar);
        complexVectorPtr++;
    }
}

#endif /* LV_HAVE_SSE */


#ifdef LV_HAVE_GENERIC

static inline void
volk_32fc_s32f_deinterleave_real_16i_generic(int16_t* iBuffer,
                                             const lv_32fc_t* complexVector,
                                             const float scalar,
                                             unsigned int num_points)
{
    const float* complexVectorPtr = (float*)complexVector;
    int16_t* iBufferPtr = iBuffer;
    unsigned int number = 0;
    for (number = 0; number < num_points; number++) {
        *iBufferPtr++ = (int16_t)rintf(*complexVectorPtr++ * scalar);
        complexVectorPtr++;
    }
}

#endif /* LV_HAVE_GENERIC */

#endif /* INCLUDED_volk_32fc_s32f_deinterleave_real_16i_a_H */

#ifndef INCLUDED_volk_32fc_s32f_deinterleave_real_16i_u_H
#define INCLUDED_volk_32fc_s32f_deinterleave_real_16i_u_H

#include <inttypes.h>
#include <stdio.h>
#include <volk/volk_common.h>

#ifdef LV_HAVE_AVX2
#include <immintrin.h>

static inline void
volk_32fc_s32f_deinterleave_real_16i_u_avx2(int16_t* iBuffer,
                                            const lv_32fc_t* complexVector,
                                            const float scalar,
                                            unsigned int num_points)
{
    unsigned int number = 0;
    const unsigned int eighthPoints = num_points / 8;

    const float* complexVectorPtr = (float*)complexVector;
    int16_t* iBufferPtr = iBuffer;

    __m256 vScalar = _mm256_set1_ps(scalar);

    __m256 cplxValue1, cplxValue2, iValue;
    __m256i a;
    __m128i b;

    __m256i idx = _mm256_set_epi32(3, 3, 3, 3, 5, 1, 4, 0);

    for (; number < eighthPoints; number++) {
        cplxValue1 = _mm256_loadu_ps(complexVectorPtr);
        complexVectorPtr += 8;

        cplxValue2 = _mm256_loadu_ps(complexVectorPtr);
        complexVectorPtr += 8;

        // Arrange in i1i2i3i4 format
        iValue = _mm256_shuffle_ps(cplxValue1, cplxValue2, _MM_SHUFFLE(2, 0, 2, 0));

        iValue = _mm256_mul_ps(iValue, vScalar);

        a = _mm256_cvtps_epi32(iValue);
        a = _mm256_packs_epi32(a, a);
        a = _mm256_permutevar8x32_epi32(a, idx);
        b = _mm256_extracti128_si256(a, 0);

        _mm_storeu_si128((__m128i*)iBufferPtr, b);
        iBufferPtr += 8;
    }

    number = eighthPoints * 8;
    iBufferPtr = &iBuffer[number];
    for (; number < num_points; number++) {
        *iBufferPtr++ = (int16_t)rintf(*complexVectorPtr++ * scalar);
        complexVectorPtr++;
    }
}

#endif /* LV_HAVE_AVX2 */

#endif /* INCLUDED_volk_32fc_s32f_deinterleave_real_16i_u_H */
