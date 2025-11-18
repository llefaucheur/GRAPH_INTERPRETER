/* Copyright (C) 2002 Jean-Marc Valin */
/**
   @file math_approx.h
   @brief Various math approximation functions for Speex
*/
/*
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

#ifndef MATH_APPROX_H
#define MATH_APPROX_H

#include "arch.h"


#define spx_sqrt sqrtf
#define spx_acos acosf
#define spx_exp expf
#define spx_cos_norm(x) (cosf((.5f*M_PI)*(x)))
#define spx_atan atanf

/** Generate a pseudo-random number */
static inline float speex_rand(float std, int32_t *seed)
{
   const unsigned int jflone = 0x3f800000;
   const unsigned int jflmsk = 0x007fffff;
   union {int i; float f;} ran;
   *seed = 1664525 * *seed + 1013904223;
   ran.i = jflone | (jflmsk & *seed);
   ran.f -= 1.5;
   return 3.4642f*std*ran.f;
}



static inline int16_t spx_ilog2(uint32_t x)
{
   int r=0;
   if (x>=(int32_t)65536)
   {
      x >>= 16;
      r += 16;
   }
   if (x>=256)
   {
      x >>= 8;
      r += 8;
   }
   if (x>=16)
   {
      x >>= 4;
      r += 4;
   }
   if (x>=4)
   {
      x >>= 2;
      r += 2;
   }
   if (x>=2)
   {
      r += 1;
   }
   return (int16_t)r;
}

static inline int16_t spx_ilog4(uint32_t x)
{
   int r=0;
   if (x>=(int32_t)65536)
   {
      x >>= 16;
      r += 8;
   }
   if (x>=256)
   {
      x >>= 8;
      r += 4;
   }
   if (x>=16)
   {
      x >>= 4;
      r += 2;
   }
   if (x>=4)
   {
      r += 1;
   }
   return (int16_t)r;
}



#ifndef M_PI
#define M_PI           3.14159265358979323846f  /* pi */
#endif

#define C1 0.9999932946f
#define C2 -0.4999124376f
#define C3 0.0414877472f
#define C4 -0.0012712095f


#define SPX_PI_2 1.5707963268f
static inline float spx_cos(float x)
{
   if (x<SPX_PI_2)
   {
      x *= x;
      return C1 + x*(C2+x*(C3+C4*x));
   } else {
      x = M_PI-x;
      x *= x;
      return NEG16(C1 + x*(C2+x*(C3+C4*x)));
   }
}

#endif
