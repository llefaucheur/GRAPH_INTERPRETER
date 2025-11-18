/* Copyright (C) 2005 Jean-Marc Valin */
/**
   @file pseudofloat.h
   @brief Pseudo-floating point
 * This header file provides a lightweight floating point type for
 * use on fixed-point platforms when a large dynamic range is
 * required. The new type is not compatible with the 32-bit IEEE format,
 * it is not even remotely as accurate as 32-bit floats, and is not
 * even guaranteed to produce even remotely correct results for code
 * other than Speex. It makes all kinds of shortcuts that are acceptable
 * for Speex, but may not be acceptable for your application. You're
 * quite welcome to reuse this code and improve it, but don't assume
 * it works out of the box. Most likely, it doesn't.
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

#ifndef PSEUDOFLOAT_H
#define PSEUDOFLOAT_H

#include "arch.h"
#include "math_approx.h"
#include <math.h>


#define spx_float_t float
#define FLOAT_ZERO 0.f
#define FLOAT_ONE 1.f
#define FLOAT_HALF 0.5f
#define PSEUDOFLOAT(x) (x)
#define FLOAT_MULT(a,b) ((a)*(b))
#define FLOAT_AMULT(a,b) ((a)*(b))
#define FLOAT_MUL32(a,b) ((a)*(b))
#define FLOAT_DIV32(a,b) ((a)/(b))
#define FLOAT_EXTRACT16(a) (a)
#define FLOAT_EXTRACT32(a) (a)
#define FLOAT_ADD(a,b) ((a)+(b))
#define FLOAT_SUB(a,b) ((a)-(b))
#define REALFLOAT(x) (x)
#define FLOAT_DIV32_FLOAT(a,b) ((a)/(b))
#define FLOAT_MUL32U(a,b) ((a)*(b))
#define FLOAT_SHL(a,b) (a)
#define FLOAT_LT(a,b) ((a)<(b))
#define FLOAT_GT(a,b) ((a)>(b))
#define FLOAT_DIVU(a,b) ((a)/(b))
#define FLOAT_SQRT(a) (spx_sqrt(a))


#endif
