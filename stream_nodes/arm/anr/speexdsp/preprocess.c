/* Copyright (C) 2003 Epic Games (written by Jean-Marc Valin)
   Copyright (C) 2004-2006 Epic Games

   File: preprocess.c
   Preprocessor with denoising based on the algorithm by Ephraim and Malah

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
   Recommended papers:

   Y. Ephraim and D. Malah, "Speech enhancement using minimum mean-square error
   short-time spectral amplitude estimator". IEEE Transactions on Acoustics,
   Speech and Signal Processing, vol. ASSP-32, no. 6, pp. 1109-1121, 1984.

   Y. Ephraim and D. Malah, "Speech enhancement using minimum mean-square error
   log-spectral amplitude estimator". IEEE Transactions on Acoustics, Speech and
   Signal Processing, vol. ASSP-33, no. 2, pp. 443-445, 1985.

   I. Cohen and B. Berdugo, "Speech enhancement for non-stationary noise environments".
   Signal Processing, vol. 81, no. 2, pp. 2403-2418, 2001.

   Stefan Gustafsson, Rainer Martin, Peter Jax, and Peter Vary. "A psychoacoustic
   approach to combined acoustic echo cancellation and noise reduction". IEEE
   Transactions on Speech and Audio Processing, 2002.

   J.-M. Valin, J. Rouat, and F. Michaud, "Microphone array post-filter for separation
   of simultaneous non-stationary sources". In Proceedings IEEE International
   Conference on Acoustics, Speech, and Signal Processing, 2004.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdint.h>
#include <math.h>
#include "speex_preprocess.h"
#include "speex_echo.h"
#include "arch.h"
#include "fftwrap.h"
#include "filterbank.h"
#include "math_approx.h"

#define LOUDNESS_EXP 5.f
#define AMP_SCALE .001f
#define AMP_SCALE_1 1000.f

#define NB_BANDS 24

#define SPEECH_PROB_START_DEFAULT       QCONST16(0.35f,15)
#define SPEECH_PROB_CONTINUE_DEFAULT    QCONST16(0.20f,15)
#define NOISE_SUPPRESS_DEFAULT       -24            // @@@

#ifndef NULL
#define NULL 0
#endif

#define SQR(x) ((x)*(x))
#define SQR16(x) (MULT16_16((x),(x)))
#define SQR16_Q15(x) (MULT16_16_Q15((x),(x)))

#define DIV32_16_Q8(a,b) ((a)/(b))
#define DIV32_16_Q15(a,b) ((a)/(b))
#define SNR_SCALING 1.f
#define SNR_SCALING_1 1.f
#define SNR_SHIFT 0
#define FRAC_SCALING 1.f
#define FRAC_SCALING_1 1.f
#define FRAC_SHIFT 0
#define NOISE_SHIFT 0

#define EXPIN_SCALING 1.f
#define EXPIN_SCALING_1 1.f
#define EXPOUT_SCALING_1 1.f


/** Speex wrapper for calloc. To do your own dynamic allocation, all you need to do is replace this function, speex_realloc and speex_free
    NOTE: speex_alloc needs to CLEAR THE MEMORY */

void* speex_alloc(uint8_t *spxGlobalHeapPtr, int size)
{
    uintptr_t ptr;
    uintptr_t previous;
    long BLOCK_MASK;
    
    BLOCK_MASK = sizeof(intptr_t) - 1;

    previous = ((uintptr_t *)spxGlobalHeapPtr)[0];
    ptr = (((uintptr_t*)spxGlobalHeapPtr)[0] + BLOCK_MASK) & ~BLOCK_MASK;

    ((uintptr_t*)spxGlobalHeapPtr)[0] = ptr + size;	    // Update pointer to next free location

    ((uintptr_t*)spxGlobalHeapPtr)[1] += ((uintptr_t*)spxGlobalHeapPtr)[0] - previous;

    return (void*)ptr;
}

/** Speex pre-processor state. */
struct SpeexPreprocessState_ {
   /* Basic info */
   int    frame_size;        /**< Number of samples processed each time */
   int    ps_size;           /**< Number of points in the power spectrum */
   int    sampling_rate;     /**< Sampling rate of the input/output */
   int    nbands;
   FilterBank *bank;

   /* Parameters */
   int    denoise_enabled;
   int    vad_enabled;
   int    dereverb_enabled;
   float  reverb_decay;
   float  reverb_level;
   float speech_prob_start;
   float speech_prob_continue;
   int    noise_suppress;

   float	speech_prob;  /**< Probability last frame was speech */

   /* DSP-related arrays */
   float *frame;      /**< Processing frame (2*ps_size) */
   float *ft;         /**< Processing frame in freq domain (2*ps_size) */
   float *ps;         /**< Current power spectrum */
   float *gain2;      /**< Adjusted gains */
   float *gain_floor; /**< Minimum gain allowed */
   float *window;     /**< Analysis/Synthesis window */
   float *noise;      /**< Noise estimate */
   //float *reverb_estimate; /**< Estimate of reverb energy */
   float *old_ps;     /**< Power spectrum for last frame */
   float *gain;       /**< Ephraim Malah gain */
   float *prior;      /**< A-priori SNR */
   float *post;       /**< A-posteriori SNR */

   float *S;          /**< Smoothed power spectrum */
   float *Smin;       /**< See Cohen paper */
   float *Stmp;       /**< See Cohen paper */
   int *update_prob;         /**< Probability of speech presence for noise update */

   float *zeta;       /**< Smoothed a priori SNR */

   /* Misc */
   float *inbuf;      /**< Input buffer (overlapped analysis) */
   float *outbuf;     /**< Output buffer (for overlap and add) */

   /* AGC stuff, only for floating point for now */
   int    agc_enabled;
   float  agc_level;
   float  loudness_accum;
   float *loudness_weight;   /**< Perceptual loudness curve */
   float  loudness;          /**< Loudness estimate */
   float  agc_gain;          /**< Current AGC gain */
   float  max_gain;          /**< Maximum gain allowed */
   float  max_increase_step; /**< Maximum increase in gain from one frame to another */
   float  max_decrease_step; /**< Maximum decrease in gain from one frame to another */
   float  prev_loudness;     /**< Loudness of previous frame */
   float  init_max;          /**< Current gain limit during initialisation */

   int    nb_adapt;          /**< Number of frames used for adaptation so far */
   int    was_speech;
   int    min_count;         /**< Number of frames processed so far */
   void  *fft_lookup;        /**< Lookup table for the FFT */
};


static void conj_window(float *w, int len)
{
   int i;
   for (i=0;i<len;i++)
   {
      float tmp;

      float x = DIV32_16(MULT16_16(QCONST16(4.f,13),i),len);
      int inv=0;
      if (x<QCONST16(1.f,13))
      {
      } else if (x<QCONST16(2.f,13))
      {
         x=QCONST16(2.f,13)-x;
         inv=1;
      } else if (x<QCONST16(3.f,13))
      {
         x=x-QCONST16(2.f,13);
         inv=1;
      } else {
         x=QCONST16(2.f,13)-x+QCONST16(2.f,13); /* 4 - x */
      }
      x = MULT16_16_Q14(QCONST16(1.271903f,14), x);
      tmp = SQR16_Q15(QCONST16(.5f,15)-MULT16_16_P15(QCONST16(.5f,15),spx_cos_norm(SHL32(EXTEND32(x),2))));
      if (inv)
         tmp=SUB16(Q15_ONE,tmp);
      w[i]=spx_sqrt(SHL32(EXTEND32(tmp),15));
   }
}



/* This function approximates the gain function
   y = gamma(1.25)^2 * M(-.25;1;-x) / sqrt(x)
   which multiplied by xi/(1+xi) is the optimal gain
   in the loudness domain ( sqrt[amplitude] )
*/
static inline float hypergeom_gain(float xx)
{
   int ind;
   float integer, frac;
   float x;
   static const float table[21] = {
      0.82157f, 1.02017f, 1.20461f, 1.37534f, 1.53363f, 1.68092f, 1.81865f,
      1.94811f, 2.07038f, 2.18638f, 2.29688f, 2.40255f, 2.50391f, 2.60144f,
      2.69551f, 2.78647f, 2.87458f, 2.96015f, 3.04333f, 3.12431f, 3.20326f};
      x = EXPIN_SCALING_1*xx;
      integer = floorf(2*x);
      ind = (int)integer;
      if (ind<0)
         return FRAC_SCALING;
      if (ind>19)
         return FRAC_SCALING*(1+.1296f/x);
      frac = 2*x-integer;
      return FRAC_SCALING*((1-frac)*table[ind] + frac*table[ind+1])/sqrtf(x+.0001f);
}

static inline float qcurve(float x)
{
   return 1.f/(1.f+.15f/(SNR_SCALING_1*x));
}

static void compute_gain_floorf(int noise_suppress, int effective_echo_suppress, float *noise, float *echo, float *gain_floor, int len)
{
   int i;
   float noise_floor;

   noise_floor = expf(.2302585f*noise_suppress);

   /* Compute the gain floor based on different floors for the background noise and residual echo */
   for (i=0;i<len;i++)
       gain_floor[i] = FRAC_SCALING * sqrtf(noise_floor * PSHR32(noise[i], NOISE_SHIFT) + 0) / sqrtf(1 + PSHR32(noise[i], NOISE_SHIFT) + 0);
}

SpeexPreprocessState *speex_preprocess_state_init(uint8_t *spxGlobalHeapPtr, int frame_size, int sampling_rate)
{
   int i;
   int N, N3, N4, M;
   extern void* speex_alloc(uint8_t *spxGlobalHeapPtr, int size);

   SpeexPreprocessState *st = (SpeexPreprocessState *)speex_alloc(spxGlobalHeapPtr, sizeof(SpeexPreprocessState));
   st->frame_size = frame_size;

   /* Round ps_size down to the nearest power of two */

   st->ps_size = st->frame_size;

   N = st->ps_size;
   N3 = 2*N - st->frame_size;
   N4 = st->frame_size - N3;

   st->sampling_rate = sampling_rate;
   st->denoise_enabled = 1;
   st->vad_enabled = 0;
   st->dereverb_enabled = 0;
   st->reverb_decay = 0;
   st->reverb_level = 0;
   st->noise_suppress = NOISE_SUPPRESS_DEFAULT;

   st->speech_prob_start = SPEECH_PROB_START_DEFAULT;
   st->speech_prob_continue = SPEECH_PROB_CONTINUE_DEFAULT;

   st->nbands = NB_BANDS;
   M = st->nbands;
   st->bank = filterbank_new(spxGlobalHeapPtr, M, (float)sampling_rate, N, 1);

   st->frame = (float*)speex_alloc(spxGlobalHeapPtr, 2*N*sizeof(float));
   st->window = (float*)speex_alloc(spxGlobalHeapPtr, 2*N*sizeof(float));
   st->ft = (float*)speex_alloc(spxGlobalHeapPtr, 2*N*sizeof(float));

   st->ps = (float*)speex_alloc(spxGlobalHeapPtr, (N+M)*sizeof(float));
   st->noise = (float*)speex_alloc(spxGlobalHeapPtr, (N+M)*sizeof(float));

   st->old_ps = (float*)speex_alloc(spxGlobalHeapPtr, (N+M)*sizeof(float));
   st->prior = (float*)speex_alloc(spxGlobalHeapPtr, (N+M)*sizeof(float));
   st->post = (float*)speex_alloc(spxGlobalHeapPtr, (N+M)*sizeof(float));
   st->gain = (float*)speex_alloc(spxGlobalHeapPtr, (N+M)*sizeof(float));
   st->gain2 = (float*)speex_alloc(spxGlobalHeapPtr, (N+M)*sizeof(float));
   st->gain_floor = (float*)speex_alloc(spxGlobalHeapPtr, (N+M)*sizeof(float));
   st->zeta = (float*)speex_alloc(spxGlobalHeapPtr, (N+M)*sizeof(float));
   st->S = (float*)speex_alloc(spxGlobalHeapPtr, N*sizeof(float));
   st->Smin = (float*)speex_alloc(spxGlobalHeapPtr, N*sizeof(float));
   st->Stmp = (float*)speex_alloc(spxGlobalHeapPtr, N*sizeof(float));
   st->update_prob = (int*)speex_alloc(spxGlobalHeapPtr, N*sizeof(int));

   st->inbuf = (float*)speex_alloc(spxGlobalHeapPtr, N3*sizeof(float));
   st->outbuf = (float*)speex_alloc(spxGlobalHeapPtr, N3*sizeof(float));

   conj_window(st->window, 2*N3);
   for (i=2*N3;i<2*st->ps_size;i++)
      st->window[i]=Q15_ONE;

   if (N4>0)
   {
      for (i=N3-1;i>=0;i--)
      {
         st->window[i+N3+N4]=st->window[i+N3];
         st->window[i+N3]=1;
      }
   }

   for (i=0;i<N+M;i++)
   {
      st->noise[i]=1;
      st->old_ps[i]=1;
      st->gain[i]=Q15_ONE;
      st->post[i]=SHL16(1, SNR_SHIFT);
      st->prior[i]=SHL16(1, SNR_SHIFT);
   }

   for (i=0;i<N;i++)
      st->update_prob[i] = 1;
   for (i=0;i<N3;i++)
   {
      st->inbuf[i]=0;
      st->outbuf[i]=0;
   }

   st->agc_enabled = 0;
   st->agc_level = 8000;
   st->loudness_weight = (float*)speex_alloc(spxGlobalHeapPtr, N*sizeof(float));
   for (i=0;i<N;i++)
   {
      float ff=((float)i)*.5f*sampling_rate/((float)N);
      /*st->loudness_weight[i] = .5f*(1.f/(1.f+ff/8000.f))+1.f*exp(-.5f*(ff-3800.f)*(ff-3800.f)/9e5f);*/
      st->loudness_weight[i] = .35f-.35f*ff/16000.f+.73f*expf(-.5f*(ff-3800)*(ff-3800)/9e5f);
      if (st->loudness_weight[i]<.01f)
         st->loudness_weight[i]=.01f;
      st->loudness_weight[i] *= st->loudness_weight[i];
   }
   /*st->loudness = powf(AMP_SCALE*st->agc_level,LOUDNESS_EXP);*/
   st->loudness = 1e-15f;
   st->agc_gain = 1;
   st->max_gain = 30;
   st->max_increase_step = expf(0.11513f * 12.f*st->frame_size / st->sampling_rate);
   st->max_decrease_step = expf(-0.11513f * 40.f*st->frame_size / st->sampling_rate);
   st->prev_loudness = 1;
   st->init_max = 1;

   st->was_speech = 0;

   st->fft_lookup = spx_fft_init(spxGlobalHeapPtr, 2*N);

   st->nb_adapt=0;
   st->min_count=0;
   return st;
}

void speex_preprocess_state_destroy(SpeexPreprocessState *st)
{
   // speex_free(st->frame);
   // speex_free(st->ft);
   // speex_free(st->ps);
   // speex_free(st->gain2);
   // speex_free(st->gain_floor);
   // speex_free(st->window);
   // speex_free(st->noise);
   // speex_free(st->reverb_estimate);
   // speex_free(st->old_ps);
   // speex_free(st->gain);
   // speex_free(st->prior);
   // speex_free(st->post);
   // speex_free(st->loudness_weight);
   // speex_free(st->echo_noise);
   // speex_free(st->residual_echo);

   // speex_free(st->S);
   // speex_free(st->Smin);
   // speex_free(st->Stmp);
   // speex_free(st->update_prob);
   // speex_free(st->zeta);

   // speex_free(st->inbuf);
   // speex_free(st->outbuf);

   // spx_fft_destroy(st->fft_lookup);
   // filterbank_destroy(st->bank);
   // speex_free(st);
}

/* FIXME: The AGC doesn't work yet with fixed-point*/
static void speex_compute_agc(SpeexPreprocessState *st, float Pframe, float *ft)
{
   int i;
   int N = st->ps_size;
   float target_gain;
   float loudness=1.f;
   float rate;

   for (i=2;i<N;i++)
   {
      loudness += 2.f*N*st->ps[i]* st->loudness_weight[i];
   }
   loudness=sqrtf(loudness);
      /*if (loudness < 2*powf(st->loudness, 1.0/LOUDNESS_EXP) &&
   loudness*2 > powf(st->loudness, 1.0/LOUDNESS_EXP))*/
   if (Pframe>.3f)
   {
      /*rate=2.0f*Pframe*Pframe/(1+st->nb_loudness_adapt);*/
      rate = .03f*Pframe*Pframe;
      st->loudness = (1-rate)*st->loudness + (rate)*powf(AMP_SCALE*loudness, LOUDNESS_EXP);
      st->loudness_accum = (1-rate)*st->loudness_accum + rate;
      if (st->init_max < st->max_gain && st->nb_adapt > 20)
         st->init_max *= 1.f + .1f*Pframe*Pframe;
   }
   /*printf ("%f %f %f %f\n", Pframe, loudness, powf(st->loudness, 1.0f/LOUDNESS_EXP), st->loudness2);*/

   target_gain = AMP_SCALE*st->agc_level*powf(st->loudness/(1e-4f+st->loudness_accum), -1.0f/LOUDNESS_EXP);

   if ((Pframe>.5  && st->nb_adapt > 20) || target_gain < st->agc_gain)
   {
      if (target_gain > st->max_increase_step*st->agc_gain)
         target_gain = st->max_increase_step*st->agc_gain;
      if (target_gain < st->max_decrease_step*st->agc_gain && loudness < 10*st->prev_loudness)
         target_gain = st->max_decrease_step*st->agc_gain;
      if (target_gain > st->max_gain)
         target_gain = st->max_gain;
      if (target_gain > st->init_max)
         target_gain = st->init_max;

      st->agc_gain = target_gain;
   }
   /*fprintf (stderr, "%f %f %f\n", loudness, (float)AMP_SCALE_1*powf(st->loudness, 1.0f/LOUDNESS_EXP), st->agc_gain);*/

   for (i=0;i<2*N;i++)
      ft[i] *= st->agc_gain;
   st->prev_loudness = loudness;
}


static void preprocess_analysis(SpeexPreprocessState *st, int16_t *x)
{
   int i;
   int N = st->ps_size;
   int N3 = 2*N - st->frame_size;
   int N4 = st->frame_size - N3;
   float *ps=st->ps;

   /* 'Build' input frame */
   for (i=0;i<N3;i++)
      st->frame[i]=st->inbuf[i];
   for (i=0;i<st->frame_size;i++)
      st->frame[N3+i]=x[i];

   /* Update inbuf */
   for (i=0;i<N3;i++)
      st->inbuf[i]=x[N4+i];

   /* Windowing */
   for (i=0;i<2*N;i++)
      st->frame[i] = MULT16_16_Q15(st->frame[i], st->window[i]);

   /* Perform FFT */
   spx_fft(st->fft_lookup, st->frame, st->ft);

   /* Power spectrum */
   ps[0]=MULT16_16(st->ft[0],st->ft[0]);
   for (i=1;i<N;i++)
      ps[i]=MULT16_16(st->ft[2*i-1],st->ft[2*i-1]) + MULT16_16(st->ft[2*i],st->ft[2*i]);
   for (i=0;i<N;i++)
      st->ps[i] = PSHR32(st->ps[i], 2*st->frame_shift);

   filterbank_compute_bank32(st->bank, ps, ps+N);
}

static void update_noise_prob(SpeexPreprocessState *st)
{
   int i;
   int min_range;
   int N = st->ps_size;

   for (i=1;i<N-1;i++)
      st->S[i] =  MULT16_32_Q15(QCONST16(.8f,15),st->S[i]) + MULT16_32_Q15(QCONST16(.05f,15),st->ps[i-1])
                      + MULT16_32_Q15(QCONST16(.1f,15),st->ps[i]) + MULT16_32_Q15(QCONST16(.05f,15),st->ps[i+1]);
   st->S[0] =  MULT16_32_Q15(QCONST16(.8f,15),st->S[0]) + MULT16_32_Q15(QCONST16(.2f,15),st->ps[0]);
   st->S[N-1] =  MULT16_32_Q15(QCONST16(.8f,15),st->S[N-1]) + MULT16_32_Q15(QCONST16(.2f,15),st->ps[N-1]);

   if (st->nb_adapt==1)
   {
      for (i=0;i<N;i++)
         st->Smin[i] = st->Stmp[i] = 0;
   }

   if (st->nb_adapt < 100)
      min_range = 15;
   else if (st->nb_adapt < 1000)
      min_range = 50;
   else if (st->nb_adapt < 10000)
      min_range = 150;
   else
      min_range = 300;
   if (st->min_count > min_range)
   {
      st->min_count = 0;
      for (i=0;i<N;i++)
      {
         st->Smin[i] = MIN32(st->Stmp[i], st->S[i]);
         st->Stmp[i] = st->S[i];
      }
   } else {
      for (i=0;i<N;i++)
      {
         st->Smin[i] = MIN32(st->Smin[i], st->S[i]);
         st->Stmp[i] = MIN32(st->Stmp[i], st->S[i]);
      }
   }
   for (i=0;i<N;i++)
   {
      if (MULT16_32_Q15(QCONST16(.4f,15),st->S[i]) > st->Smin[i])
         st->update_prob[i] = 1;
      else
         st->update_prob[i] = 0;
      /*fprintf (stderr, "%f ", st->S[i]/st->Smin[i]);*/
      /*fprintf (stderr, "%f ", st->update_prob[i]);*/
   }

}

#define NOISE_OVERCOMPENS 1.

int speex_preprocess(SpeexPreprocessState *st, int16_t *x)
{
   return speex_preprocess_run(st, x);
}

int speex_preprocess_run(SpeexPreprocessState *st, int16_t *x)
{
   int i;
   int M;
   int N = st->ps_size;
   int N3 = 2*N - st->frame_size;
   int N4 = st->frame_size - N3;
   float *ps=st->ps;
   float Zframe;
   float Pframe;
   float beta, beta_1;

   st->nb_adapt++;
   if (st->nb_adapt>20000)
      st->nb_adapt = 20000;
   st->min_count++;

   beta = MAX16(QCONST16(.03f,15),DIV32_16(Q15_ONE,st->nb_adapt));
   beta_1 = Q15_ONE-beta;
   M = st->nbands;

   preprocess_analysis(st, x);

   update_noise_prob(st);

   /* Noise estimation always updated for the 10 first frames */
   /*if (st->nb_adapt<10)
   {
      for (i=1;i<N-1;i++)
         st->update_prob[i] = 0;
   }
   */

   /* Update the noise estimate for the frequencies where it can be */
   for (i=0;i<N;i++)
   {
      if (!st->update_prob[i] || st->ps[i] < PSHR32(st->noise[i], NOISE_SHIFT))
         st->noise[i] = MAX32(EXTEND32(0),MULT16_32_Q15(beta_1,st->noise[i]) + MULT16_32_Q15(beta,SHL32(st->ps[i],NOISE_SHIFT)));
   }
   filterbank_compute_bank32(st->bank, st->noise, st->noise+N);

   /* Special case for first frame */
   if (st->nb_adapt==1)
      for (i=0;i<N+M;i++)
         st->old_ps[i] = ps[i];

   /* Compute a posteriori SNR */
   for (i=0;i<N+M;i++)
   {
      float gamma;

      /* Total noise estimate including residual echo and reverberation */
      float tot_noise = ADD32(ADD32(ADD32(EXTEND32(1), PSHR32(st->noise[i],NOISE_SHIFT)) , 0) , 0);

      /* A posteriori SNR = ps/noise - 1*/
      st->post[i] = SUB16(DIV32_16_Q8(ps[i],tot_noise), QCONST16(1.f,SNR_SHIFT));
      st->post[i]=MIN16(st->post[i], QCONST16(100.f,SNR_SHIFT));

      /* Computing update gamma = .1 + .9*(old/(old+noise))^2 */
      gamma = QCONST16(.1f,15)+MULT16_16_Q15(QCONST16(.89f,15),SQR16_Q15(DIV32_16_Q15(st->old_ps[i],ADD32(st->old_ps[i],tot_noise))));

      /* A priori SNR update = gamma*max(0,post) + (1-gamma)*old/noise */
      st->prior[i] = EXTRACT16(PSHR32(ADD32(MULT16_16(gamma,MAX16(0,st->post[i])), MULT16_16(Q15_ONE-gamma,DIV32_16_Q8(st->old_ps[i],tot_noise))), 15));
      st->prior[i]=MIN16(st->prior[i], QCONST16(100.f,SNR_SHIFT));
   }

   /*print_vec(st->post, N+M, "");*/

   /* Recursive average of the a priori SNR. A bit smoothed for the psd components */
   st->zeta[0] = PSHR32(ADD32(MULT16_16(QCONST16(.7f,15),st->zeta[0]), MULT16_16(QCONST16(.3f,15),st->prior[0])),15);
   for (i=1;i<N-1;i++)
      st->zeta[i] = PSHR32(ADD32(ADD32(ADD32(MULT16_16(QCONST16(.7f,15),st->zeta[i]), MULT16_16(QCONST16(.15f,15),st->prior[i])),
                           MULT16_16(QCONST16(.075f,15),st->prior[i-1])), MULT16_16(QCONST16(.075f,15),st->prior[i+1])),15);
   for (i=N-1;i<N+M;i++)
      st->zeta[i] = PSHR32(ADD32(MULT16_16(QCONST16(.7f,15),st->zeta[i]), MULT16_16(QCONST16(.3f,15),st->prior[i])),15);

   /* Speech probability of presence for the entire frame is based on the average filterbank a priori SNR */
   Zframe = 0;
   for (i=N;i<N+M;i++)
      Zframe = ADD32(Zframe, EXTEND32(st->zeta[i]));
   Pframe = QCONST16(.1f,15)+MULT16_16_Q15(QCONST16(.899f,15),qcurve(DIV32_16(Zframe,st->nbands)));

   compute_gain_floorf(st->noise_suppress, 0, st->noise+N, 0, st->gain_floor+N, M);

   /* Compute Ephraim & Malah gain speech probability of presence for each critical band (Bark scale)
      Technically this is actually wrong because the EM gaim assumes a slightly different probability
      distribution */
   for (i=N;i<N+M;i++)
   {
      /* See EM and Cohen papers*/
      float theta;
      /* Gain from hypergeometric function */
      float MM;
      /* Weiner filter gain */
      float prior_ratio;
      /* a priority probability of speech presence based on Bark sub-band alone */
      float P1;
      /* Speech absence a priori probability (considering sub-band and frame) */
      float q;

      prior_ratio = PDIV32_16(SHL32(EXTEND32(st->prior[i]), 15), ADD16(st->prior[i], SHL32(1,SNR_SHIFT)));
      theta = MULT16_32_P15(prior_ratio, QCONST32(1.f,EXPIN_SHIFT)+SHL32(EXTEND32(st->post[i]),EXPIN_SHIFT-SNR_SHIFT));

      MM = hypergeom_gain(theta);
      /* Gain with bound */
      st->gain[i] = EXTRACT16(MIN32(Q15_ONE, MULT16_32_Q15(prior_ratio, MM)));
      /* Save old Bark power spectrum */
      st->old_ps[i] = MULT16_32_P15(QCONST16(.2f,15),st->old_ps[i]) + MULT16_32_P15(MULT16_16_P15(QCONST16(.8f,15),SQR16_Q15(st->gain[i])),ps[i]);

      P1 = QCONST16(.199f,15)+MULT16_16_Q15(QCONST16(.8f,15),qcurve (st->zeta[i]));
      q = Q15_ONE-MULT16_16_Q15(Pframe,P1);

      st->gain2[i]=1.f/(1.f + (q/(1.f-q))*(1+st->prior[i])*expf(-theta));

   }
   /* Convert the EM gains and speech prob to linear frequency */
   filterbank_compute_psd16(st->bank,st->gain2+N, st->gain2);
   filterbank_compute_psd16(st->bank,st->gain+N, st->gain);

   /* Use 1 for linear gain resolution (best) or 0 for Bark gain resolution (faster) */
   if (1)
   {
      filterbank_compute_psd16(st->bank,st->gain_floor+N, st->gain_floor);

      /* Compute gain according to the Ephraim-Malah algorithm -- linear frequency */
      for (i=0;i<N;i++)
      {
         float MM;
         float theta;
         float prior_ratio;
         float tmp;
         float p;
         float g;

         /* Wiener filter gain */
         prior_ratio = PDIV32_16(SHL32(EXTEND32(st->prior[i]), 15), ADD16(st->prior[i], SHL32(1,SNR_SHIFT)));
         theta = MULT16_32_P15(prior_ratio, QCONST32(1.f,EXPIN_SHIFT)+SHL32(EXTEND32(st->post[i]),EXPIN_SHIFT-SNR_SHIFT));

         /* Optimal estimator for loudness domain */
         MM = hypergeom_gain(theta);
         /* EM gain with bound */
         g = EXTRACT16(MIN32(Q15_ONE, MULT16_32_Q15(prior_ratio, MM)));
         /* Interpolated speech probability of presence */
         p = st->gain2[i];

         /* Constrain the gain to be close to the Bark scale gain */
         if (MULT16_16_Q15(QCONST16(.333f,15),g) > st->gain[i])
            g = MULT16_16(3,st->gain[i]);
         st->gain[i] = g;

         /* Save old power spectrum */
         st->old_ps[i] = MULT16_32_P15(QCONST16(.2f,15),st->old_ps[i]) + MULT16_32_P15(MULT16_16_P15(QCONST16(.8f,15),SQR16_Q15(st->gain[i])),ps[i]);

         /* Apply gain floor */
         if (st->gain[i] < st->gain_floor[i])
            st->gain[i] = st->gain_floor[i];

         /* Exponential decay model for reverberation (unused) */
         /*st->reverb_estimate[i] = st->reverb_decay*st->reverb_estimate[i] + st->reverb_decay*st->reverb_level*st->gain[i]*st->gain[i]*st->ps[i];*/

         /* Take into account speech probability of presence (loudness domain MMSE estimator) */
         /* gain2 = [p*sqrt(gain)+(1-p)*sqrt(gain _floor) ]^2 */
         tmp = MULT16_16_P15(p,spx_sqrt(SHL32(EXTEND32(st->gain[i]),15))) + MULT16_16_P15(SUB16(Q15_ONE,p),spx_sqrt(SHL32(EXTEND32(st->gain_floor[i]),15)));
         st->gain2[i]=SQR16_Q15(tmp);

         /* Use this if you want a log-domain MMSE estimator instead */
         /*st->gain2[i] = powf(st->gain[i], p) * powf(st->gain_floor[i],1.f-p);*/
      }
   } else {
      for (i=N;i<N+M;i++)
      {
         float tmp;
         float p = st->gain2[i];
         st->gain[i] = MAX16(st->gain[i], st->gain_floor[i]);
         tmp = MULT16_16_P15(p,spx_sqrt(SHL32(EXTEND32(st->gain[i]),15))) + MULT16_16_P15(SUB16(Q15_ONE,p),spx_sqrt(SHL32(EXTEND32(st->gain_floor[i]),15)));
         st->gain2[i]=SQR16_Q15(tmp);
      }
      filterbank_compute_psd16(st->bank,st->gain2+N, st->gain2);
   }

   /* If noise suppression is off, don't apply the gain (but then why call this in the first place!) */
   if (!st->denoise_enabled)
   {
      for (i=0;i<N+M;i++)
         st->gain2[i]=Q15_ONE;
   }

   /* Apply computed gain */
   for (i=1;i<N;i++)
   {
      st->ft[2*i-1] = MULT16_16_P15(st->gain2[i],st->ft[2*i-1]);
      st->ft[2*i] = MULT16_16_P15(st->gain2[i],st->ft[2*i]);
   }
   st->ft[0] = MULT16_16_P15(st->gain2[0],st->ft[0]);
   st->ft[2*N-1] = MULT16_16_P15(st->gain2[N-1],st->ft[2*N-1]);

   /*FIXME: This *will* not work for fixed-point */

   if (st->agc_enabled)
      speex_compute_agc(st, Pframe, st->ft);


   /* Inverse FFT with 1/N scaling */
   spx_ifft(st->fft_lookup, st->ft, st->frame);
   /* Scale back to original (lower) amplitude */
   for (i=0;i<2*N;i++)
      st->frame[i] = PSHR16(st->frame[i], st->frame_shift);

   /*FIXME: This *will* not work for fixed-point */

   if (st->agc_enabled)
   {
      float max_sample=0;
      for (i=0;i<2*N;i++)
         if (fabs(st->frame[i])>max_sample)
            max_sample = fabsf(st->frame[i]);
      if (max_sample>28000.f)
      {
         float damp = 28000.f/max_sample;
         for (i=0;i<2*N;i++)
            st->frame[i] *= damp;
      }
   }


   /* Synthesis window (for WOLA) */
   for (i=0;i<2*N;i++)
      st->frame[i] = MULT16_16_Q15(st->frame[i], st->window[i]);

   /* Perform overlap and add */
   for (i=0;i<N3;i++)
      x[i] = WORD2INT(ADD32(EXTEND32(st->outbuf[i]), EXTEND32(st->frame[i])));
   for (i=0;i<N4;i++)
      x[N3+i] = (int16_t)(st->frame[N3+i]);

   /* Update outbuf */
   for (i=0;i<N3;i++)
      st->outbuf[i] = st->frame[st->frame_size+i];

   /* FIXME: This VAD is a kludge */
   st->speech_prob = Pframe;
   if (st->vad_enabled)
   {
      if (st->speech_prob > st->speech_prob_start || (st->was_speech && st->speech_prob > st->speech_prob_continue))
      {
         st->was_speech=1;
         return 1;
      } else
      {
         st->was_speech=0;
         return 0;
      }
   } else {
      return 1;
   }
}

void speex_preprocess_estimate_update(SpeexPreprocessState *st, int16_t *x)
{
   int i;
   int N = st->ps_size;
   int N3 = 2*N - st->frame_size;
   int M;
   float *ps=st->ps;

   M = st->nbands;
   st->min_count++;

   preprocess_analysis(st, x);

   update_noise_prob(st);

   for (i=1;i<N-1;i++)
   {
      if (!st->update_prob[i] || st->ps[i] < PSHR32(st->noise[i],NOISE_SHIFT))
      {
         st->noise[i] = MULT16_32_Q15(QCONST16(.95f,15),st->noise[i]) + MULT16_32_Q15(QCONST16(.05f,15),SHL32(st->ps[i],NOISE_SHIFT));
      }
   }

   for (i=0;i<N3;i++)
      st->outbuf[i] = MULT16_16_Q15(x[st->frame_size-N3+i],st->window[st->frame_size+i]);

   /* Save old power spectrum */
   for (i=0;i<N+M;i++)
      st->old_ps[i] = ps[i];

   //for (i=0;i<N;i++)
   //   st->reverb_estimate[i] = MULT16_32_Q15(st->reverb_decay, st->reverb_estimate[i]);
}


int speex_preprocess_ctl(SpeexPreprocessState *state, int request, void *ptr)
{
   int i;
   SpeexPreprocessState *st;
   st=(SpeexPreprocessState*)state;
   switch(request)
   {
   case SPEEX_PREPROCESS_SET_DENOISE:
      st->denoise_enabled = (*(int32_t*)ptr);
      break;
   case SPEEX_PREPROCESS_GET_DENOISE:
      (*(int32_t*)ptr) = st->denoise_enabled;
      break;

   case SPEEX_PREPROCESS_SET_AGC:
      st->agc_enabled = (*(int32_t*)ptr);
      break;
   case SPEEX_PREPROCESS_GET_AGC:
      (*(int32_t*)ptr) = st->agc_enabled;
      break;
#ifndef DISABLE_FLOAT_API
   case SPEEX_PREPROCESS_SET_AGC_LEVEL:
      st->agc_level = (*(float*)ptr);
      if (st->agc_level<1)
         st->agc_level=1;
      if (st->agc_level>32768)
         st->agc_level=32768;
      break;
   case SPEEX_PREPROCESS_GET_AGC_LEVEL:
      (*(float*)ptr) = st->agc_level;
      break;
#endif /* #ifndef DISABLE_FLOAT_API */
   case SPEEX_PREPROCESS_SET_AGC_INCREMENT:
      st->max_increase_step = expf(0.11513f * (*(int32_t*)ptr)*st->frame_size / st->sampling_rate);
      break;
   case SPEEX_PREPROCESS_GET_AGC_INCREMENT:
      (*(int32_t*)ptr) = (int32_t)floorf(.5f+8.6858f*logf(st->max_increase_step)*st->sampling_rate/st->frame_size);
      break;
   case SPEEX_PREPROCESS_SET_AGC_DECREMENT:
      st->max_decrease_step = expf(0.11513f * (*(int32_t*)ptr)*st->frame_size / st->sampling_rate);
      break;
   case SPEEX_PREPROCESS_GET_AGC_DECREMENT:
      (*(int32_t*)ptr) = (int32_t)floorf(.5f+8.6858f*logf(st->max_decrease_step)*st->sampling_rate/st->frame_size);
      break;
   case SPEEX_PREPROCESS_SET_AGC_MAX_GAIN:
      st->max_gain = expf(0.11513f * (*(int32_t*)ptr));
      break;
   case SPEEX_PREPROCESS_GET_AGC_MAX_GAIN:
      (*(int32_t*)ptr) = (int32_t)floorf(.5f+8.6858f*logf(st->max_gain));
      break;

   case SPEEX_PREPROCESS_SET_VAD:
      //speex_warning("The VAD has been replaced by a hack pending a complete rewrite");
      st->vad_enabled = (*(int32_t*)ptr);
      break;
   case SPEEX_PREPROCESS_GET_VAD:
      (*(int32_t*)ptr) = st->vad_enabled;
      break;

   case SPEEX_PREPROCESS_SET_DEREVERB:
       st->dereverb_enabled = 0; // (*(int32_t*)ptr);
      //for (i=0;i<st->ps_size;i++)
      //   st->reverb_estimate[i]=0;
      break;
   case SPEEX_PREPROCESS_GET_DEREVERB:
      (*(int32_t*)ptr) = st->dereverb_enabled;
      break;

   case SPEEX_PREPROCESS_SET_DEREVERB_LEVEL:
      /* FIXME: Re-enable when de-reverberation is actually enabled again */
      /*st->reverb_level = (*(float*)ptr);*/
      break;
   case SPEEX_PREPROCESS_GET_DEREVERB_LEVEL:
      /* FIXME: Re-enable when de-reverberation is actually enabled again */
      /*(*(float*)ptr) = st->reverb_level;*/
      break;

   case SPEEX_PREPROCESS_SET_DEREVERB_DECAY:
      /* FIXME: Re-enable when de-reverberation is actually enabled again */
      /*st->reverb_decay = (*(float*)ptr);*/
      break;
   case SPEEX_PREPROCESS_GET_DEREVERB_DECAY:
      /* FIXME: Re-enable when de-reverberation is actually enabled again */
      /*(*(float*)ptr) = st->reverb_decay;*/
      break;

   case SPEEX_PREPROCESS_SET_PROB_START:
      *(int32_t*)ptr = MIN32(100,MAX32(0, *(int32_t*)ptr));
      st->speech_prob_start = DIV32_16(MULT16_16(Q15ONE,*(int32_t*)ptr), 100);
      break;
   case SPEEX_PREPROCESS_GET_PROB_START:
      (*(int32_t*)ptr) = (int32_t)MULT16_16_Q15(st->speech_prob_start, 100);
      break;

   case SPEEX_PREPROCESS_SET_PROB_CONTINUE:
      *(int32_t*)ptr = MIN32(100,MAX32(0, *(int32_t*)ptr));
      st->speech_prob_continue = DIV32_16(MULT16_16(Q15ONE,*(int32_t*)ptr), 100);
      break;
   case SPEEX_PREPROCESS_GET_PROB_CONTINUE:
      (*(int32_t*)ptr) = (int32_t)MULT16_16_Q15(st->speech_prob_continue, 100);
      break;

   case SPEEX_PREPROCESS_SET_NOISE_SUPPRESS:
      st->noise_suppress = -ABS(*(int32_t*)ptr);
      break;
   case SPEEX_PREPROCESS_GET_NOISE_SUPPRESS:
      (*(int32_t*)ptr) = st->noise_suppress;
      break;

   case SPEEX_PREPROCESS_GET_AGC_LOUDNESS:
      (*(int32_t*)ptr) = (int32_t)powf(st->loudness, 1.0f/LOUDNESS_EXP);
      break;
   case SPEEX_PREPROCESS_GET_AGC_GAIN:
      (*(int32_t*)ptr) = (int32_t)floorf(.5f+8.6858f*logf(st->agc_gain));
      break;

   case SPEEX_PREPROCESS_GET_PSD_SIZE:
   case SPEEX_PREPROCESS_GET_NOISE_PSD_SIZE:
      (*(int32_t*)ptr) = st->ps_size;
      break;
   case SPEEX_PREPROCESS_GET_PSD:
      for(i=0;i<st->ps_size;i++)
      	((int32_t *)ptr)[i] = (int32_t) st->ps[i];
      break;
   case SPEEX_PREPROCESS_GET_NOISE_PSD:
      for(i=0;i<st->ps_size;i++)
      	((int32_t *)ptr)[i] = (int32_t) PSHR32(st->noise[i], NOISE_SHIFT);
      break;
   case SPEEX_PREPROCESS_GET_PROB:
      (*(int32_t*)ptr) = (int32_t)MULT16_16_Q15(st->speech_prob, 100);
      break;

   case SPEEX_PREPROCESS_SET_AGC_TARGET:
      st->agc_level = (*(float*)ptr);
      if (st->agc_level<1)
         st->agc_level=1;
      if (st->agc_level>32768)
         st->agc_level=32768;
      break;
   case SPEEX_PREPROCESS_GET_AGC_TARGET:
       (*(int32_t*)ptr) = (int32_t)(st->agc_level);
      break;
   default:
      //speex_warning_int("Unknown speex_preprocess_ctl request: ", request);
      return -1;
   }
   return 0;
}

#ifdef FIXED_DEBUG
long long spx_mips=0;
#endif

