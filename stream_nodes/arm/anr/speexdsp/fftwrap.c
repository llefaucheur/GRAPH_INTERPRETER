/* Copyright (C) 2005 Jean-Marc Valin
   File: fftwrap.h

   Wrapper for various FFTs

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions
   are met:

   - Redistributions of source code must retain the above copyright
   notice, this list of conditions and the following disclaimer.

   - Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   - Neither the name of the Xiph.org Foundation nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
   ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
   A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR
   CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#ifndef FFTWRAP_H
#define FFTWRAP_H

#include "arch.h"


/** Compute tables for an FFT */
void* spx_fft_init(uint8_t *spxGlobalHeapPtr, int size);

/** Forward (real to half-complex) transform */
void spx_fft(void* table, float* in, float* out);

/** Backward (half-complex to real) transform */
void spx_ifft(void* table, float* in, float* out);


#define MAX_FFT_SIZE 2048
#include "Include/arm_math.h"

struct cmsis_fft_config {
    arm_rfft_fast_instance_f32 inst;
    /* need copy as CMSIS DSP rFFT corrupts input */
    float* scratchIn;
    /* need another copy as CMSIS DSP rFFT issue unwanted symmetric part */
    float* scratchOut;
#endif
    int  N;
};

void* spx_fft_init(uint8_t *spxGlobalHeapPtr, int size)
{
    extern void* speex_alloc(uint8_t *spxGlobalHeapPtr, int size);
    struct cmsis_fft_config* table;
    table = (struct cmsis_fft_config*)speex_alloc(spxGlobalHeapPtr, sizeof(struct cmsis_fft_config));

    table->scratchIn = (float*)speex_alloc(spxGlobalHeapPtr, size * 2 * sizeof(float));
    table->scratchOut = (float*)speex_alloc(spxGlobalHeapPtr, size * 2 * sizeof(float));
    //speex_assert(table->scratchIn != NULL);
    //speex_assert(table->scratchOut != NULL);

    arm_rfft_fast_init_f32(&table->inst, (uint16_t)size);

    table->N = size;
    return table;
}


void memcopy_32(const float32_t* pSrc, float32_t* pDst, uint32_t blockSize)
{   uint32_t i;
    for (i = 0; i < blockSize; i++) *pDst++ = *pSrc++;
}

void memcopy_scale_32(const float32_t* pSrc, float32_t scale, float32_t* pDst, uint32_t blockSize)
{   uint32_t i;
    for (i = 0; i < blockSize; i++) *pDst++ = *pSrc++ * scale;
}

void spx_fft(void* table, float* in, float* out)
{
    struct cmsis_fft_config* t = (struct cmsis_fft_config*)table;

    int N = t->N;
    float* scratchIn = t->scratchIn;
    float* scratchOut = t->scratchOut;


    /* copy to avoid RFFT input corruption */
    memcopy_32(in, scratchIn, N);
    arm_rfft_fast_f32(&t->inst, scratchIn, scratchOut, 0);

    /* CMSIS DSP to libspeex float RFFT reshufling and rescaling */
    out[0] = scratchOut[0] / (float)N;
    out[N - 1] = scratchOut[1] / (float)N;
    memcopy_scale_32(scratchOut + 2, 1.0f / (float)N, out + 1, N - 2);
}

void spx_ifft(void* table, float* in, float* out)
{
    struct cmsis_fft_config* t = (struct cmsis_fft_config*)table;
    float* scratchIn = t->scratchIn;
    float* scratchOut = t->scratchOut;
    int N = t->N;

    /* CMSIS DSP RFFT float reshuffling */
    memcopy_32(in + 1, scratchIn + 2, (N - 2) * sizeof(float));
    scratchIn[0] = in[0];
    scratchIn[1] = in[N - 1];

    arm_rfft_fast_f32(&t->inst, scratchIn, scratchOut, 1);
    /* CMSIS RIFFT scale down, need to compensate */
    memcopy_scale_32(scratchOut, (float)N, out, N);

}

