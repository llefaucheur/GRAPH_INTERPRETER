/* Copyright (C) 2003-2008 Jean-Marc Valin

   File: mdf.c
   Echo canceller based on the MDF algorithm (see below)

   Redistribution and use in source and binary forms, with or without
   modification, are permitted provided that the following conditions are
   met:

   1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright
   notice, this list of conditions and the following disclaimer in the
   documentation and/or other materials provided with the distribution.

   3. The name of the author may not be used to endorse or promote products
   derived from this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
   IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
   OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
   INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
   HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
   STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
   ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
   POSSIBILITY OF SUCH DAMAGE.
*/

/*
   The echo canceller is based on the MDF algorithm described in:

   J. S. Soo, K. K. Pang Multidelay block frequency adaptive filter,
   IEEE Trans. Acoust. Speech Signal Process., Vol. ASSP-38, No. 2,
   February 1990.

   We use the Alternatively Updated MDF (AUMDF) variant. Robustness to
   double-talk is achieved using a variable learning rate as described in:

   Valin, J.-M., On Adjusting the Learning Rate in Frequency Domain Echo
   Cancellation With Double-Talk. IEEE Transactions on Audio,
   Speech and Language Processing, Vol. 15, No. 3, pp. 1030-1034, 2007.
   http://people.xiph.org/~jm/papers/valin_taslp2006.pdf

   There is no explicit double-talk detection, but a continuous variation
   in the learning rate based on residual echo, double-talk and background
   noise.

   About the fixed-point version:
   All the signals are represented with 16-bit words. The filter weights
   are represented with 32-bit words, but only the top 16 bits are used
   in most cases. The lower 16 bits are completely unreliable (due to the
   fact that the update is done only on the top bits), but help in the
   adaptation -- probably by removing a "threshold effect" due to
   quantization (rounding going to zero) when the gradient is small.

   Another kludge that seems to work good: when performing the weight
   update, we only move half the way toward the "goal" this seems to
   reduce the effect of quantization noise in the update phase. This
   can be seen as applying a gradient descent on a "soft constraint"
   instead of having a hard constraint.

*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "arch.h"
#include "speex_echo.h"
#include "fftwrap.h"
#include "pseudofloat.h"
#include "math_approx.h"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif



/* If enabled, the AEC will use a foreground filter and a background filter to be more robust to double-talk
   and difficult signals in general. The cost is an extra FFT and a matrix-vector multiply */
//#define TWO_PATH

static const spx_float_t MIN_LEAK = .005f;

/* Constants for the two-path filter */
static const spx_float_t VAR1_SMOOTH = .36f;
static const spx_float_t VAR2_SMOOTH = .7225f;
static const spx_float_t VAR1_UPDATE = .5f;
static const spx_float_t VAR2_UPDATE = .25f;
static const spx_float_t VAR_BACKTRACK = 4.f;
#define TOP16(x) (x)


#define PLAYBACK_DELAY 2

void speex_echo_get_residual(SpeexEchoState *st, float *Yout, int len);


/** Speex echo cancellation state. */
struct SpeexEchoState_ {
   int frame_size;           /**< Number of samples processed each time */
   int window_size;
   int M;
   int cancel_count;
   int adapted;
   int saturated;
   int screwed_up;
   int C;                    /** Number of input channels (microphones) */
   int K;                    /** Number of output channels (loudspeakers) */
   int32_t sampling_rate;
   float spec_average;
   float beta0;
   float beta_max;
   float sum_adapt;
   float leak_estimate;

   float *e;      /* scratch */
   float *x;      /* Far-end input buffer (2N) */
   float *X;      /* Far-end buffer (M+1 frames) in frequency domain */
   float *input;  /* scratch */
   float *y;      /* scratch */
   float *last_y;
   float *Y;      /* scratch */
   float *E;
   float *PHI;    /* scratch */
   float *W;      /* (Background) filter weights */

   float *foreground; /* Foreground filter weights */
   float  Davg1;  /* 1st recursive average of the residual power difference */
   float  Davg2;  /* 2nd recursive average of the residual power difference */
   spx_float_t   Dvar1;  /* Estimated variance of 1st estimator */
   spx_float_t   Dvar2;  /* Estimated variance of 2nd estimator */

   float *power;  /* Power of the far-end signal */
   spx_float_t  *power_1;/* Inverse power of far-end */
   float *wtmp;   /* scratch */

   float *Rf;     /* scratch */
   float *Yf;     /* scratch */
   float *Xf;     /* scratch */
   float *Eh;
   float *Yh;
   spx_float_t   Pey;
   spx_float_t   Pyy;
   float *window;
   float *prop;
   void *fft_table;
   float *memX, *memD, *memE;
   float preemph;
   float notch_radius;
   float *notch_mem;

   /* NOTE: If you only use speex_echo_cancel() and want to save some memory, remove this */
   int16_t *play_buf;
   int play_buf_pos;
   int play_buf_started;
};

static inline void filter_dc_notch16(const int16_t *in, float radius, float *out, int len, float *mem, int stride)
{
   int i;
   float den2;

   den2 = radius*radius + .7f*(1-radius)*(1-radius);

   /*printf ("%d %d %d %d %d %d\n", num[0], num[1], num[2], den[0], den[1], den[2]);*/
   for (i=0;i<len;i++)
   {
      float vin = in[i*stride];
      float vout = mem[0] + SHL32(EXTEND32(vin),15);

      mem[0] = mem[1] + 2*(-vin + radius*vout);

      mem[1] = SHL32(EXTEND32(vin),15) - MULT16_32_Q15(den2,vout);
      out[i] = SATURATE32(PSHR32(MULT16_32_Q15(radius,vout),15),32767);
   }
}

/* This inner product is slightly different from the codec version because of fixed-point */
static inline float mdf_inner_prod(const float *x, const float *y, int len)
{
   float sum=0;
   len >>= 1;
   while(len--)
   {
      float part=0;
      part = MAC16_16(part,*x++,*y++);
      part = MAC16_16(part,*x++,*y++);
      /* HINT: If you had a 40-bit accumulator, you could shift only at the end */
      sum = ADD32(sum,SHR32(part,6));
   }
   return sum;
}

/** Compute power spectrum of a half-complex (packed) vector */
static inline void power_spectrum(const float *X, float *ps, int N)
{
   int i, j;
   ps[0]=MULT16_16(X[0],X[0]);
   for (i=1,j=1;i<N-1;i+=2,j++)
   {
      ps[j] =  MULT16_16(X[i],X[i]) + MULT16_16(X[i+1],X[i+1]);
   }
   ps[j]=MULT16_16(X[i],X[i]);
}

/** Compute power spectrum of a half-complex (packed) vector and accumulate */
static inline void power_spectrum_accum(const float *X, float *ps, int N)
{
   int i, j;
   ps[0]+=MULT16_16(X[0],X[0]);
   for (i=1,j=1;i<N-1;i+=2,j++)
   {
      ps[j] +=  MULT16_16(X[i],X[i]) + MULT16_16(X[i+1],X[i+1]);
   }
   ps[j]+=MULT16_16(X[i],X[i]);
}


static inline void spectral_mul_accum(const float *X, const float *Y, float *acc, int N, int M)
{
   int i,j;
   for (i=0;i<N;i++)
      acc[i] = 0;
   for (j=0;j<M;j++)
   {
      acc[0] += X[0]*Y[0];
      for (i=1;i<N-1;i+=2)
      {
         acc[i] += (X[i]*Y[i] - X[i+1]*Y[i+1]);
         acc[i+1] += (X[i+1]*Y[i] + X[i]*Y[i+1]);
      }
      acc[i] += X[i]*Y[i];
      X += N;
      Y += N;
   }
}
#define spectral_mul_accum16 spectral_mul_accum

/** Compute weighted cross-power spectrum of a half-complex (packed) vector with conjugate */
static inline void weighted_spectral_mul_conj(const spx_float_t *w, const spx_float_t p, const float *X, const float *Y, float *prod, int N)
{
   int i, j;
   spx_float_t W;
   W = FLOAT_AMULT(p, w[0]);
   prod[0] = FLOAT_MUL32(W,MULT16_16(X[0],Y[0]));
   for (i=1,j=1;i<N-1;i+=2,j++)
   {
      W = FLOAT_AMULT(p, w[j]);
      prod[i] = FLOAT_MUL32(W,MAC16_16(MULT16_16(X[i],Y[i]), X[i+1],Y[i+1]));
      prod[i+1] = FLOAT_MUL32(W,MAC16_16(MULT16_16(-X[i+1],Y[i]), X[i],Y[i+1]));
   }
   W = FLOAT_AMULT(p, w[j]);
   prod[i] = FLOAT_MUL32(W,MULT16_16(X[i],Y[i]));
}

static inline void mdf_adjust_prop(const float *W, int N, int M, int P, float *prop)
{
   int i, j, p;
   float max_sum = 1;
   float prop_sum = 1;
   for (i=0;i<M;i++)
   {
      float tmp = 1;
      for (p=0;p<P;p++)
         for (j=0;j<N;j++)
            tmp += MULT16_16(EXTRACT16(SHR32(W[p*N*M + i*N+j],18)), EXTRACT16(SHR32(W[p*N*M + i*N+j],18)));

      prop[i] = spx_sqrt(tmp);
      if (prop[i] > max_sum)
         max_sum = prop[i];
   }
   for (i=0;i<M;i++)
   {
      prop[i] += MULT16_16_Q15(QCONST16(.1f,15),max_sum);
      prop_sum += EXTEND32(prop[i]);
   }
   for (i=0;i<M;i++)
   {
      prop[i] = DIV32(MULT16_16(QCONST16(.99f,15), prop[i]),prop_sum);
      /*printf ("%f ", prop[i]);*/
   }
   /*printf ("\n");*/
}

/** Creates a new echo canceller state */
SpeexEchoState *speex_echo_state_init(uint8_t *spxGlobalHeapPtr, int frame_size, int filter_length)
{
   return speex_echo_state_init_mc(spxGlobalHeapPtr, frame_size, filter_length, 1, 1);
}

SpeexEchoState *speex_echo_state_init_mc(uint8_t *spxGlobalHeapPtr, int frame_size, int filter_length, int nb_mic, int nb_speakers)
{
   extern void* speex_alloc(uint8_t *spxGlobalHeapPtr, int size);
   int i,N,M, C, K;
   SpeexEchoState *st = (SpeexEchoState *)speex_alloc(spxGlobalHeapPtr, sizeof(SpeexEchoState));

   st->K = nb_speakers;
   st->C = nb_mic;
   C=st->C;
   K=st->K;

   st->frame_size = frame_size;
   st->window_size = 2*frame_size;
   N = st->window_size;
   M = st->M = (filter_length+st->frame_size-1)/frame_size;
   st->cancel_count=0;
   st->sum_adapt = 0;
   st->saturated = 0;
   st->screwed_up = 0;
   /* This is the default sampling rate */
   st->sampling_rate = 8000;
   st->spec_average = DIV32_16(SHL32(EXTEND32(st->frame_size), 15), st->sampling_rate);

   st->beta0 = (2.0f*st->frame_size)/st->sampling_rate;
   st->beta_max = (.5f*st->frame_size)/st->sampling_rate;

   st->leak_estimate = 0;

   st->fft_table = spx_fft_init(spxGlobalHeapPtr, N);

   st->e = (float*)speex_alloc(spxGlobalHeapPtr, C*N*sizeof(float));
   st->x = (float*)speex_alloc(spxGlobalHeapPtr, K*N*sizeof(float));
   st->input = (float*)speex_alloc(spxGlobalHeapPtr, C*st->frame_size*sizeof(float));
   st->y = (float*)speex_alloc(spxGlobalHeapPtr, C*N*sizeof(float));
   st->last_y = (float*)speex_alloc(spxGlobalHeapPtr, C*N*sizeof(float));
   st->Yf = (float*)speex_alloc(spxGlobalHeapPtr, (st->frame_size+1)*sizeof(float));
   st->Rf = (float*)speex_alloc(spxGlobalHeapPtr, (st->frame_size+1)*sizeof(float));
   st->Xf = (float*)speex_alloc(spxGlobalHeapPtr, (st->frame_size+1)*sizeof(float));
   st->Yh = (float*)speex_alloc(spxGlobalHeapPtr, (st->frame_size+1)*sizeof(float));
   st->Eh = (float*)speex_alloc(spxGlobalHeapPtr, (st->frame_size+1)*sizeof(float));

   st->X = (float*)speex_alloc(spxGlobalHeapPtr, K*(M+1)*N*sizeof(float));
   st->Y = (float*)speex_alloc(spxGlobalHeapPtr, C*N*sizeof(float));
   st->E = (float*)speex_alloc(spxGlobalHeapPtr, C*N*sizeof(float));
   st->W = (float*)speex_alloc(spxGlobalHeapPtr, C*K*M*N*sizeof(float));

   st->foreground = (float*)speex_alloc(spxGlobalHeapPtr, M*N*C*K*sizeof(float));

   st->PHI = (float*)speex_alloc(spxGlobalHeapPtr, N*sizeof(float));
   st->power = (float*)speex_alloc(spxGlobalHeapPtr, (frame_size+1)*sizeof(float));
   st->power_1 = (spx_float_t*)speex_alloc(spxGlobalHeapPtr, (frame_size+1)*sizeof(spx_float_t));
   st->window = (float*)speex_alloc(spxGlobalHeapPtr, N*sizeof(float));
   st->prop = (float*)speex_alloc(spxGlobalHeapPtr, M*sizeof(float));
   st->wtmp = (float*)speex_alloc(spxGlobalHeapPtr, N*sizeof(float));

   for (i=0;i<N;i++)
      st->window[i] = .5f-.5f*cosf(2*M_PI*i/N);

   for (i=0;i<=st->frame_size;i++)
      st->power_1[i] = FLOAT_ONE;
   for (i=0;i<N*M*K*C;i++)
      st->W[i] = 0;
   {
      float sum = 0;
      /* Ratio of ~10 between adaptation rate of first and last block */
      float decay = SHR32(spx_exp(NEG16(DIV32_16(QCONST16(2.4f,11),M))),1);
      st->prop[0] = QCONST16(.7f, 15);
      sum = EXTEND32(st->prop[0]);
      for (i=1;i<M;i++)
      {
         st->prop[i] = MULT16_16_Q15(st->prop[i-1], decay);
         sum = ADD32(sum, EXTEND32(st->prop[i]));
      }
      for (i=M-1;i>=0;i--)
      {
         st->prop[i] = DIV32(MULT16_16(QCONST16(.8f,15), st->prop[i]),sum);
      }
   }

   st->memX = (float*)speex_alloc(spxGlobalHeapPtr, K*sizeof(float));
   st->memD = (float*)speex_alloc(spxGlobalHeapPtr, C*sizeof(float));
   st->memE = (float*)speex_alloc(spxGlobalHeapPtr, C*sizeof(float));
   st->preemph = QCONST16(.9f,15);
   if (st->sampling_rate<12000)
      st->notch_radius = QCONST16(.9f, 15);
   else if (st->sampling_rate<24000)
      st->notch_radius = QCONST16(.982f, 15);
   else
      st->notch_radius = QCONST16(.992f, 15);

   st->notch_mem = (float*)speex_alloc(spxGlobalHeapPtr, 2*C*sizeof(float));
   st->adapted = 0;
   st->Pey = st->Pyy = FLOAT_ONE;

   st->Davg1 = st->Davg2 = 0;
   st->Dvar1 = st->Dvar2 = FLOAT_ZERO;

   st->play_buf = (int16_t*)speex_alloc(spxGlobalHeapPtr, K*(PLAYBACK_DELAY+1)*st->frame_size*sizeof(int16_t));
   st->play_buf_pos = PLAYBACK_DELAY*st->frame_size;
   st->play_buf_started = 0;

   return st;
}

/** Resets echo canceller state */
void speex_echo_state_reset(SpeexEchoState *st)
{
   int i, M, N, C, K;
   st->cancel_count=0;
   st->screwed_up = 0;
   N = st->window_size;
   M = st->M;
   C=st->C;
   K=st->K;
   for (i=0;i<N*M;i++)
      st->W[i] = 0;

   for (i=0;i<N*M;i++)
      st->foreground[i] = 0;

   for (i=0;i<N*(M+1);i++)
      st->X[i] = 0;
   for (i=0;i<=st->frame_size;i++)
   {
      st->power[i] = 0;
      st->power_1[i] = FLOAT_ONE;
      st->Eh[i] = 0;
      st->Yh[i] = 0;
   }
   for (i=0;i<st->frame_size;i++)
   {
      st->last_y[i] = 0;
   }
   for (i=0;i<N*C;i++)
   {
      st->E[i] = 0;
   }
   for (i=0;i<N*K;i++)
   {
      st->x[i] = 0;
   }
   for (i=0;i<2*C;i++)
      st->notch_mem[i] = 0;
   for (i=0;i<C;i++)
      st->memD[i]=st->memE[i]=0;
   for (i=0;i<K;i++)
      st->memX[i]=0;

   st->saturated = 0;
   st->adapted = 0;
   st->sum_adapt = 0;
   st->Pey = st->Pyy = FLOAT_ONE;

   st->Davg1 = st->Davg2 = 0;
   st->Dvar1 = st->Dvar2 = FLOAT_ZERO;

   for (i=0;i<3*st->frame_size;i++)
      st->play_buf[i] = 0;
   st->play_buf_pos = PLAYBACK_DELAY*st->frame_size;
   st->play_buf_started = 0;

}

///** Destroys an echo canceller state */
//void speex_echo_state_destroy(SpeexEchoState *st)
//{
//   spx_fft_destroy(st->fft_table);
//
//   speex_free(st->e);
//   speex_free(st->x);
//   speex_free(st->input);
//   speex_free(st->y);
//   speex_free(st->last_y);
//   speex_free(st->Yf);
//   speex_free(st->Rf);
//   speex_free(st->Xf);
//   speex_free(st->Yh);
//   speex_free(st->Eh);
//
//   speex_free(st->X);
//   speex_free(st->Y);
//   speex_free(st->E);
//   speex_free(st->W);
//
//   speex_free(st->foreground);
//
//   speex_free(st->PHI);
//   speex_free(st->power);
//   speex_free(st->power_1);
//   speex_free(st->window);
//   speex_free(st->prop);
//   speex_free(st->wtmp);
//
//   speex_free(st->memX);
//   speex_free(st->memD);
//   speex_free(st->memE);
//   speex_free(st->notch_mem);
//
//   speex_free(st->play_buf);
//   speex_free(st);
//}

//void speex_echo_capture(SpeexEchoState *st, const int16_t *rec, int16_t *out)
//{
//   int i;
//   /*speex_warning_int("capture with fill level ", st->play_buf_pos/st->frame_size);*/
//   st->play_buf_started = 1;
//   if (st->play_buf_pos>=st->frame_size)
//   {
//      speex_echo_cancellation(st, rec, st->play_buf, out);
//      st->play_buf_pos -= st->frame_size;
//      for (i=0;i<st->play_buf_pos;i++)
//         st->play_buf[i] = st->play_buf[i+st->frame_size];
//   } else {
//      //speex_warning("No playback frame available (your application is buggy and/or got xruns)");
//      if (st->play_buf_pos!=0)
//      {
//         speex_warning("internal playback buffer corruption?");
//         st->play_buf_pos = 0;
//      }
//      for (i=0;i<st->frame_size;i++)
//         out[i] = rec[i];
//   }
//}

void speex_echo_playback(SpeexEchoState *st, const int16_t *play)
{
   /*speex_warning_int("playback with fill level ", st->play_buf_pos/st->frame_size);*/
   if (!st->play_buf_started)
   {
      //speex_warning("discarded first playback frame");
      return;
   }
   if (st->play_buf_pos<=PLAYBACK_DELAY*st->frame_size)
   {
      int i;
      for (i=0;i<st->frame_size;i++)
         st->play_buf[st->play_buf_pos+i] = play[i];
      st->play_buf_pos += st->frame_size;
      if (st->play_buf_pos <= (PLAYBACK_DELAY-1)*st->frame_size)
      {
         //speex_warning("Auto-filling the buffer (your application is buggy and/or got xruns)");
         for (i=0;i<st->frame_size;i++)
            st->play_buf[st->play_buf_pos+i] = play[i];
         st->play_buf_pos += st->frame_size;
      }
   } else {
      //speex_warning("Had to discard a playback frame (your application is buggy and/or got xruns)");
   }
}
//
///** Performs echo cancellation on a frame (deprecated, last arg now ignored) */
//void speex_echo_cancel(SpeexEchoState *st, const int16_t *in, const int16_t *far_end, int16_t *out, int32_t *Yout)
//{
//   speex_echo_cancellation(st, in, far_end, out);
//}
//
///** Performs echo cancellation on a frame */
//void speex_echo_cancellation(SpeexEchoState *st, const int16_t *in, const int16_t *far_end, int16_t *out)
//{
//   int i,j, chan, speak;
//   int N,M, C, K;
//   float Syy,See,Sxx,Sdd, Sff;
//
//   float Dbf;
//   int update_foreground;
//
//   float Sey;
//   float ss, ss_1;
//   spx_float_t Pey = FLOAT_ONE, Pyy=FLOAT_ONE;
//   spx_float_t alpha, alpha_1;
//   float RER;
//   float tmp32;
//
//   N = st->window_size;
//   M = st->M;
//   C = st->C;
//   K = st->K;
//
//   st->cancel_count++;
//
//   ss=.35f/M;
//   ss_1 = 1-ss;
//
//
//   for (chan = 0; chan < C; chan++)
//   {
//      /* Apply a notch filter to make sure DC doesn't end up causing problems */
//      filter_dc_notch16(in+chan, st->notch_radius, st->input+chan*st->frame_size, st->frame_size, st->notch_mem+2*chan, C);
//      /* Copy input data to buffer and apply pre-emphasis */
//      /* Copy input data to buffer */
//      for (i=0;i<st->frame_size;i++)
//      {
//         float tmp32;
//         /* FIXME: This core has changed a bit, need to merge properly */
//         tmp32 = SUB32(EXTEND32(st->input[chan*st->frame_size+i]), EXTEND32(MULT16_16_P15(st->preemph, st->memD[chan])));
//
//         st->memD[chan] = st->input[chan*st->frame_size+i];
//         st->input[chan*st->frame_size+i] = EXTRACT16(tmp32);
//      }
//   }
//
//   for (speak = 0; speak < K; speak++)
//   {
//      for (i=0;i<st->frame_size;i++)
//      {
//         float tmp32;
//         st->x[speak*N+i] = st->x[speak*N+i+st->frame_size];
//         tmp32 = SUB32(EXTEND32(far_end[i*K+speak]), EXTEND32(MULT16_16_P15(st->preemph, st->memX[speak])));
//
//         st->x[speak*N+i+st->frame_size] = EXTRACT16(tmp32);
//         st->memX[speak] = far_end[i*K+speak];
//      }
//   }
//
//   for (speak = 0; speak < K; speak++)
//   {
//      /* Shift memory: this could be optimized eventually*/
//      for (j=M-1;j>=0;j--)
//      {
//         for (i=0;i<N;i++)
//            st->X[(j+1)*N*K+speak*N+i] = st->X[j*N*K+speak*N+i];
//      }
//      /* Convert x (echo input) to frequency domain */
//      spx_fft(st->fft_table, st->x+speak*N, &st->X[speak*N]);
//   }
//
//   Sxx = 0;
//   for (speak = 0; speak < K; speak++)
//   {
//      Sxx += mdf_inner_prod(st->x+speak*N+st->frame_size, st->x+speak*N+st->frame_size, st->frame_size);
//      power_spectrum_accum(st->X+speak*N, st->Xf, N);
//   }
//
//   Sff = 0;
//   for (chan = 0; chan < C; chan++)
//   {
//
//      /* Compute foreground filter */
//      spectral_mul_accum16(st->X, st->foreground+chan*N*K*M, st->Y+chan*N, N, M*K);
//      spx_ifft(st->fft_table, st->Y+chan*N, st->e+chan*N);
//      for (i=0;i<st->frame_size;i++)
//         st->e[chan*N+i] = SUB16(st->input[chan*st->frame_size+i], st->e[chan*N+i+st->frame_size]);
//      Sff += mdf_inner_prod(st->e+chan*N, st->e+chan*N, st->frame_size);
//
//   }
//
//   /* Adjust proportional adaption rate */
//   /* FIXME: Adjust that for C, K*/
//   if (st->adapted)
//      mdf_adjust_prop (st->W, N, M, C*K, st->prop);
//   /* Compute weight gradient */
//   if (st->saturated == 0)
//   {
//      for (chan = 0; chan < C; chan++)
//      {
//         for (speak = 0; speak < K; speak++)
//         {
//            for (j=M-1;j>=0;j--)
//            {
//               weighted_spectral_mul_conj(st->power_1, FLOAT_SHL(PSEUDOFLOAT(st->prop[j]),-15), &st->X[(j+1)*N*K+speak*N], st->E+chan*N, st->PHI, N);
//               for (i=0;i<N;i++)
//                  st->W[chan*N*K*M + j*N*K + speak*N + i] += st->PHI[i];
//            }
//         }
//      }
//   } else {
//      st->saturated--;
//   }
//
//   /* FIXME: MC conversion required */
//   /* Update weight to prevent circular convolution (MDF / AUMDF) */
//   for (chan = 0; chan < C; chan++)
//   {
//      for (speak = 0; speak < K; speak++)
//      {
//         for (j=0;j<M;j++)
//         {
//            /* This is a variant of the Alternatively Updated MDF (AUMDF) */
//            /* Remove the "if" to make this an MDF filter */
//            if (j==0 || st->cancel_count%(M-1) == j-1)
//            {
//
//               spx_ifft(st->fft_table, &st->W[chan*N*K*M + j*N*K + speak*N], st->wtmp);
//               for (i=st->frame_size;i<N;i++)
//               {
//                  st->wtmp[i]=0;
//               }
//               spx_fft(st->fft_table, st->wtmp, &st->W[chan*N*K*M + j*N*K + speak*N]);
//
//            }
//         }
//      }
//   }
//
//   /* So we can use power_spectrum_accum */
//   for (i=0;i<=st->frame_size;i++)
//      st->Rf[i] = st->Yf[i] = st->Xf[i] = 0;
//
//   Dbf = 0;
//   See = 0;
//
//   /* Difference in response, this is used to estimate the variance of our residual power estimate */
//   for (chan = 0; chan < C; chan++)
//   {
//      spectral_mul_accum(st->X, st->W+chan*N*K*M, st->Y+chan*N, N, M*K);
//      spx_ifft(st->fft_table, st->Y+chan*N, st->y+chan*N);
//      for (i=0;i<st->frame_size;i++)
//         st->e[chan*N+i] = SUB16(st->e[chan*N+i+st->frame_size], st->y[chan*N+i+st->frame_size]);
//      Dbf += 10+mdf_inner_prod(st->e+chan*N, st->e+chan*N, st->frame_size);
//      for (i=0;i<st->frame_size;i++)
//         st->e[chan*N+i] = SUB16(st->input[chan*st->frame_size+i], st->y[chan*N+i+st->frame_size]);
//      See += mdf_inner_prod(st->e+chan*N, st->e+chan*N, st->frame_size);
//   }
//
//
//   /* Logic for updating the foreground filter */
//
//   /* For two time windows, compute the mean of the energy difference, as well as the variance */
//   st->Davg1 = ADD32(MULT16_32_Q15(QCONST16(.6f,15),st->Davg1), MULT16_32_Q15(QCONST16(.4f,15),SUB32(Sff,See)));
//   st->Davg2 = ADD32(MULT16_32_Q15(QCONST16(.85f,15),st->Davg2), MULT16_32_Q15(QCONST16(.15f,15),SUB32(Sff,See)));
//   st->Dvar1 = FLOAT_ADD(FLOAT_MULT(VAR1_SMOOTH, st->Dvar1), FLOAT_MUL32U(MULT16_32_Q15(QCONST16(.4f,15),Sff), MULT16_32_Q15(QCONST16(.4f,15),Dbf)));
//   st->Dvar2 = FLOAT_ADD(FLOAT_MULT(VAR2_SMOOTH, st->Dvar2), FLOAT_MUL32U(MULT16_32_Q15(QCONST16(.15f,15),Sff), MULT16_32_Q15(QCONST16(.15f,15),Dbf)));
//
//   /* Equivalent float code:
//   st->Davg1 = .6*st->Davg1 + .4*(Sff-See);
//   st->Davg2 = .85*st->Davg2 + .15*(Sff-See);
//   st->Dvar1 = .36*st->Dvar1 + .16*Sff*Dbf;
//   st->Dvar2 = .7225*st->Dvar2 + .0225*Sff*Dbf;
//   */
//
//   update_foreground = 0;
//   /* Check if we have a statistically significant reduction in the residual echo */
//   /* Note that this is *not* Gaussian, so we need to be careful about the longer tail */
//   if (FLOAT_GT(FLOAT_MUL32U(SUB32(Sff,See),ABS32(SUB32(Sff,See))), FLOAT_MUL32U(Sff,Dbf)))
//      update_foreground = 1;
//   else if (FLOAT_GT(FLOAT_MUL32U(st->Davg1, ABS32(st->Davg1)), FLOAT_MULT(VAR1_UPDATE,(st->Dvar1))))
//      update_foreground = 1;
//   else if (FLOAT_GT(FLOAT_MUL32U(st->Davg2, ABS32(st->Davg2)), FLOAT_MULT(VAR2_UPDATE,(st->Dvar2))))
//      update_foreground = 1;
//
//   /* Do we update? */
//   if (update_foreground)
//   {
//      st->Davg1 = st->Davg2 = 0;
//      st->Dvar1 = st->Dvar2 = FLOAT_ZERO;
//      /* Copy background filter to foreground filter */
//      for (i=0;i<N*M*C*K;i++)
//         st->foreground[i] = EXTRACT16(PSHR32(st->W[i],16));
//      /* Apply a smooth transition so as to not introduce blocking artifacts */
//      for (chan = 0; chan < C; chan++)
//         for (i=0;i<st->frame_size;i++)
//            st->e[chan*N+i+st->frame_size] = MULT16_16_Q15(st->window[i+st->frame_size],st->e[chan*N+i+st->frame_size]) + MULT16_16_Q15(st->window[i],st->y[chan*N+i+st->frame_size]);
//   } else {
//      int reset_background=0;
//      /* Otherwise, check if the background filter is significantly worse */
//      if (FLOAT_GT(FLOAT_MUL32U(NEG32(SUB32(Sff,See)),ABS32(SUB32(Sff,See))), FLOAT_MULT(VAR_BACKTRACK,FLOAT_MUL32U(Sff,Dbf))))
//         reset_background = 1;
//      if (FLOAT_GT(FLOAT_MUL32U(NEG32(st->Davg1), ABS32(st->Davg1)), FLOAT_MULT(VAR_BACKTRACK,st->Dvar1)))
//         reset_background = 1;
//      if (FLOAT_GT(FLOAT_MUL32U(NEG32(st->Davg2), ABS32(st->Davg2)), FLOAT_MULT(VAR_BACKTRACK,st->Dvar2)))
//         reset_background = 1;
//      if (reset_background)
//      {
//         /* Copy foreground filter to background filter */
//         for (i=0;i<N*M*C*K;i++)
//            st->W[i] = SHL32(EXTEND32(st->foreground[i]),16);
//         /* We also need to copy the output so as to get correct adaptation */
//         for (chan = 0; chan < C; chan++)
//         {
//            for (i=0;i<st->frame_size;i++)
//               st->y[chan*N+i+st->frame_size] = st->e[chan*N+i+st->frame_size];
//            for (i=0;i<st->frame_size;i++)
//               st->e[chan*N+i] = SUB16(st->input[chan*st->frame_size+i], st->y[chan*N+i+st->frame_size]);
//         }
//         See = Sff;
//         st->Davg1 = st->Davg2 = 0;
//         st->Dvar1 = st->Dvar2 = FLOAT_ZERO;
//      }
//   }
//
//
//   Sey = Syy = Sdd = 0;
//   for (chan = 0; chan < C; chan++)
//   {
//      /* Compute error signal (for the output with de-emphasis) */
//      for (i=0;i<st->frame_size;i++)
//      {
//         float tmp_out;
//
//         tmp_out = SUB32(EXTEND32(st->input[chan*st->frame_size+i]), EXTEND32(st->e[chan*N+i+st->frame_size]));
//
//         tmp_out = ADD32(tmp_out, EXTEND32(MULT16_16_P15(st->preemph, st->memE[chan])));
//      /* This is an arbitrary test for saturation in the microphone signal */
//         if (in[i*C+chan] <= -32000 || in[i*C+chan] >= 32000)
//         {
//         if (st->saturated == 0)
//            st->saturated = 1;
//         }
//         out[i*C+chan] = WORD2INT(tmp_out);
//         st->memE[chan] = tmp_out;
//      }
//
//      /* Compute error signal (filter update version) */
//      for (i=0;i<st->frame_size;i++)
//      {
//         st->e[chan*N+i+st->frame_size] = st->e[chan*N+i];
//         st->e[chan*N+i] = 0;
//      }
//
//      /* Compute a bunch of correlations */
//      /* FIXME: bad merge */
//      Sey += mdf_inner_prod(st->e+chan*N+st->frame_size, st->y+chan*N+st->frame_size, st->frame_size);
//      Syy += mdf_inner_prod(st->y+chan*N+st->frame_size, st->y+chan*N+st->frame_size, st->frame_size);
//      Sdd += mdf_inner_prod(st->input+chan*st->frame_size, st->input+chan*st->frame_size, st->frame_size);
//
//      /* Convert error to frequency domain */
//      spx_fft(st->fft_table, st->e+chan*N, st->E+chan*N);
//      for (i=0;i<st->frame_size;i++)
//         st->y[i+chan*N] = 0;
//      spx_fft(st->fft_table, st->y+chan*N, st->Y+chan*N);
//
//      /* Compute power spectrum of echo (X), error (E) and filter response (Y) */
//      power_spectrum_accum(st->E+chan*N, st->Rf, N);
//      power_spectrum_accum(st->Y+chan*N, st->Yf, N);
//
//   }
//
//   /*printf ("%f %f %f %f\n", Sff, See, Syy, Sdd, st->update_cond);*/
//
//   /* Do some sanity check */
//   if (!(Syy>=0 && Sxx>=0 && See >= 0)
//
//      )
//   {
//      /* Things have gone really bad */
//      st->screwed_up += 50;
//      for (i=0;i<st->frame_size*C;i++)
//         out[i] = 0;
//   } else if (SHR32(Sff, 2) > ADD32(Sdd, SHR32(MULT16_16(N, 10000),6)))
//   {
//      /* AEC seems to add lots of echo instead of removing it, let's see if it will improve */
//      st->screwed_up++;
//   } else {
//      /* Everything's fine */
//      st->screwed_up=0;
//   }
//   if (st->screwed_up>=50)
//   {
//      speex_warning("The echo canceller started acting funny and got slapped (reset). It swears it will behave now.");
//      speex_echo_state_reset(st);
//      return;
//   }
//
//   /* Add a small noise floor to make sure not to have problems when dividing */
//   See = MAX32(See, SHR32(MULT16_16(N, 100),6));
//
//   for (speak = 0; speak < K; speak++)
//   {
//      Sxx += mdf_inner_prod(st->x+speak*N+st->frame_size, st->x+speak*N+st->frame_size, st->frame_size);
//      power_spectrum_accum(st->X+speak*N, st->Xf, N);
//   }
//
//
//   /* Smooth far end energy estimate over time */
//   for (j=0;j<=st->frame_size;j++)
//      st->power[j] = MULT16_32_Q15(ss_1,st->power[j]) + 1 + MULT16_32_Q15(ss,st->Xf[j]);
//
//   /* Compute filtered spectra and (cross-)correlations */
//   for (j=st->frame_size;j>=0;j--)
//   {
//      spx_float_t Eh, Yh;
//      Eh = PSEUDOFLOAT(st->Rf[j] - st->Eh[j]);
//      Yh = PSEUDOFLOAT(st->Yf[j] - st->Yh[j]);
//      Pey = FLOAT_ADD(Pey,FLOAT_MULT(Eh,Yh));
//      Pyy = FLOAT_ADD(Pyy,FLOAT_MULT(Yh,Yh));
//
//      st->Eh[j] = (1-st->spec_average)*st->Eh[j] + st->spec_average*st->Rf[j];
//      st->Yh[j] = (1-st->spec_average)*st->Yh[j] + st->spec_average*st->Yf[j];
//
//   }
//
//   Pyy = FLOAT_SQRT(Pyy);
//   Pey = FLOAT_DIVU(Pey,Pyy);
//
//   /* Compute correlation updatete rate */
//   tmp32 = MULT16_32_Q15(st->beta0,Syy);
//   if (tmp32 > MULT16_32_Q15(st->beta_max,See))
//      tmp32 = MULT16_32_Q15(st->beta_max,See);
//   alpha = FLOAT_DIV32(tmp32, See);
//   alpha_1 = FLOAT_SUB(FLOAT_ONE, alpha);
//   /* Update correlations (recursive average) */
//   st->Pey = FLOAT_ADD(FLOAT_MULT(alpha_1,st->Pey) , FLOAT_MULT(alpha,Pey));
//   st->Pyy = FLOAT_ADD(FLOAT_MULT(alpha_1,st->Pyy) , FLOAT_MULT(alpha,Pyy));
//   if (FLOAT_LT(st->Pyy, FLOAT_ONE))
//      st->Pyy = FLOAT_ONE;
//   /* We don't really hope to get better than 33 dB (MIN_LEAK-3dB) attenuation anyway */
//   if (FLOAT_LT(st->Pey, FLOAT_MULT(MIN_LEAK,st->Pyy)))
//      st->Pey = FLOAT_MULT(MIN_LEAK,st->Pyy);
//   if (FLOAT_GT(st->Pey, st->Pyy))
//      st->Pey = st->Pyy;
//   /* leak_estimate is the linear regression result */
//   st->leak_estimate = FLOAT_EXTRACT16(FLOAT_SHL(FLOAT_DIVU(st->Pey, st->Pyy),14));
//   /* This looks like a stupid bug, but it's right (because we convert from Q14 to Q15) */
//   if (st->leak_estimate > 16383)
//      st->leak_estimate = 32767;
//   else
//      st->leak_estimate = SHL16(st->leak_estimate,1);
//   /*printf ("%f\n", st->leak_estimate);*/
//
//   /* Compute Residual to Error Ratio */
//
//   RER = (.0001f*Sxx + 3.f*MULT16_32_Q15(st->leak_estimate,Syy)) / See;
//   /* Check for y in e (lower bound on RER) */
//   if (RER < Sey*Sey/(1+See*Syy))
//      RER = Sey*Sey/(1+See*Syy);
//   if (RER > .5)
//      RER = .5;
//
//
//   /* We consider that the filter has had minimal adaptation if the following is true*/
//   if (!st->adapted && st->sum_adapt > SHL32(EXTEND32(M),15) && MULT16_32_Q15(st->leak_estimate,Syy) > MULT16_32_Q15(QCONST16(.03f,15),Syy))
//   {
//      st->adapted = 1;
//   }
//
//   if (st->adapted)
//   {
//      /* Normal learning rate calculation once we're past the minimal adaptation phase */
//      for (i=0;i<=st->frame_size;i++)
//      {
//         float r, e;
//         /* Compute frequency-domain adaptation mask */
//         r = MULT16_32_Q15(st->leak_estimate,SHL32(st->Yf[i],3));
//         e = SHL32(st->Rf[i],3)+1;
//
//         if (r>.5*e)
//            r = .5f*e;
//
//         r = MULT16_32_Q15(QCONST16(.7f,15),r) + MULT16_32_Q15(QCONST16(.3f,15),(float)(MULT16_32_Q15(RER,e)));
//         /*st->power_1[i] = adapt_rate*r/(e*(1+st->power[i]));*/
//         st->power_1[i] = FLOAT_SHL(FLOAT_DIV32_FLOAT(r,FLOAT_MUL32U(e,st->power[i]+10)),WEIGHT_SHIFT+16);
//      }
//   } else {
//      /* Temporary adaption rate if filter is not yet adapted enough */
//      float adapt_rate=0;
//
//      if (Sxx > SHR32(MULT16_16(N, 1000),6))
//      {
//         tmp32 = MULT16_32_Q15(QCONST16(.25f, 15), Sxx);
//
//         if (tmp32 > .25f*See)
//            tmp32 = .25f*See;
//
//         adapt_rate = FLOAT_EXTRACT16(FLOAT_SHL(FLOAT_DIV32(tmp32, See),15));
//      }
//      for (i=0;i<=st->frame_size;i++)
//         st->power_1[i] = FLOAT_SHL(FLOAT_DIV32(EXTEND32(adapt_rate),ADD32(st->power[i],10)),WEIGHT_SHIFT+1);
//
//
//      /* How much have we adapted so far? */
//      st->sum_adapt = ADD32(st->sum_adapt,adapt_rate);
//   }
//
//   /* FIXME: MC conversion required */
//      for (i=0;i<st->frame_size;i++)
//         st->last_y[i] = st->last_y[st->frame_size+i];
//   if (st->adapted)
//   {
//      /* If the filter is adapted, take the filtered echo */
//      for (i=0;i<st->frame_size;i++)
//         st->last_y[st->frame_size+i] = (float)(in[i]-out[i]);
//   } else {
//      /* If filter isn't adapted yet, all we can do is take the far end signal directly */
//      /* moved earlier: for (i=0;i<N;i++)
//      st->last_y[i] = st->x[i];*/
//   }
//
//}
//
///* Compute spectrum of estimated echo for use in an echo post-filter */
//void speex_echo_get_residual(SpeexEchoState *st, float *residual_echo, int len)
//{
//   int i;
//   float leak2;
//   int N;
//
//   N = st->window_size;
//
//   /* Apply hanning window (should pre-compute it)*/
//   for (i=0;i<N;i++)
//      st->y[i] = MULT16_16_Q15(st->window[i],st->last_y[i]);
//
//   /* Compute power spectrum of the echo */
//   spx_fft(st->fft_table, st->y, st->Y);
//   power_spectrum(st->Y, residual_echo, N);
//
//
//   if (st->leak_estimate>.5)
//      leak2 = 1;
//   else
//      leak2 = 2*st->leak_estimate;
//
//   /* Estimate residual echo */
//   for (i=0;i<=st->frame_size;i++)
//      residual_echo[i] = MULT16_32_Q15(leak2,residual_echo[i]);
//
//}

//int speex_echo_ctl(SpeexEchoState *st, int request, void *ptr)
//{
//   switch(request)
//   {
//
//      case SPEEX_ECHO_GET_FRAME_SIZE:
//         (*(int*)ptr) = st->frame_size;
//         break;
//      case SPEEX_ECHO_SET_SAMPLING_RATE:
//         st->sampling_rate = (*(int*)ptr);
//         st->spec_average = DIV32_16(SHL32(EXTEND32(st->frame_size), 15), st->sampling_rate);
//
//         st->beta0 = (2.0f*st->frame_size)/st->sampling_rate;
//         st->beta_max = (.5f*st->frame_size)/st->sampling_rate;
//
//         if (st->sampling_rate<12000)
//            st->notch_radius = QCONST16(.9f, 15);
//         else if (st->sampling_rate<24000)
//            st->notch_radius = QCONST16(.982f, 15);
//         else
//            st->notch_radius = QCONST16(.992f, 15);
//         break;
//      case SPEEX_ECHO_GET_SAMPLING_RATE:
//         (*(int*)ptr) = st->sampling_rate;
//         break;
//      case SPEEX_ECHO_GET_IMPULSE_RESPONSE_SIZE:
//         /*FIXME: Implement this for multiple channels */
//         *((int32_t *)ptr) = st->M * st->frame_size;
//         break;
//      case SPEEX_ECHO_GET_IMPULSE_RESPONSE:
//      {
//         int M = st->M, N = st->window_size, n = st->frame_size, i, j;
//         int32_t *filt = (int32_t *) ptr;
//         for(j=0;j<M;j++)
//         {
//            /*FIXME: Implement this for multiple channels */
//
//            spx_ifft(st->fft_table, &st->W[j*N], st->wtmp);
//
//            for(i=0;i<n;i++)
//               filt[j*n+i] = (int32_t)PSHR32(MULT16_16(32767,st->wtmp[i]), WEIGHT_SHIFT-NORMALIZE_SCALEDOWN);
//         }
//      }
//         break;
//      default:
//         speex_warning_int("Unknown speex_echo_ctl request: ", request);
//         return -1;
//   }
//   return 0;
//}
