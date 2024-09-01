/* ----------------------------------------------------------------------
 * Title:        sigp_stream_converter_process.c
 * Description:  filters
 *
 * $Date:        15 February 2024
 * $Revision:    V0.0.1
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2013-2024 signal-processing.fr. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 */

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "sigp_stream_converter.h"


/**
  @brief         deinterleaving and channel mapping, check raw_in, domain_in, nchan(in/out) and intlv 
  @param[in]     in_out    XDM buffer
  @return        none
 */


/*
  - copy input data to the scratch buffer with the management of the target number of channels and using 
    deinterleave intermediate format. 
    The start of the deinterleaved buffer is a copy of the previously processed end of buffers, 
    for the implementation of filters using a memory.

*/
void sigp_stream_converter_process1 (sigp_stream_converter_format *input, sigp_stream_converter_format *output, 
    float *inBuf, float *tmpBuf1)
{
    
input->framesize       ;
input->nchan           ;
input->raw             ;
input->interleaving    ;
input->timestamp       ;
input->timestampsize   ;
input->sampling_rate   ;
input->mapping         ;
}

/*----------------------------------------------------------------------------*
 *
 * Function    : vlib_dotp_m4_f32()
 *
 * Description : dot-product benchmark for more than 24 taps 
 *
 * parameters  : two pointers to vectors
 *
 * Returns     : the sum (dot-product)
 *
 * Complexity  :  N/A
 *
 -----------------------------------------------------------------------------*/

static f32_t dotp_f32_16taps (f32_t *x, f32_t *h, int ntaps)
{
    //f32_t sum;
   	int16_t i;
    f32_t sum64;

#if NEON_ENABLE

   	int16x4_t h_vec, x_vec;
   	int32x4_t result_vec;

	/* Clear the scalar and vector sums */
    result_vec = vdupq_n_s32(0);

    h_vec = vld1_s16(h);
    x_vec = vld1_s16(x);
    result_vec = vmlal_s16(result_vec, h_vec, x_vec);

    h_vec = vld1_s16(h + 4);
    x_vec = vld1_s16(x + 4);
    result_vec = vmlal_s16(result_vec, h_vec, x_vec);

    h_vec = vld1_s16(h + 8);
    x_vec = vld1_s16(x + 8);
    result_vec = vmlal_s16(result_vec, h_vec, x_vec);

    h_vec = vld1_s16(h + 12);
    x_vec = vld1_s16(x + 12);
    result_vec = vmlal_s16(result_vec, h_vec, x_vec);

    h_vec = vld1_s16(h + 16);
    x_vec = vld1_s16(x + 16);
    result_vec = vmlal_s16(result_vec, h_vec, x_vec);

    h_vec = vld1_s16(h + 20);
    x_vec = vld1_s16(x + 20);
    result_vec = vmlal_s16(result_vec, h_vec, x_vec);

    if (ntaps < 64 && ntap > 24)
    { switch (ntaps)
      {
                   h_vec = vld1_s16(h + 56);
                   x_vec = vld1_s16(x + 56);
                   result_vec = vmlal_s16(result_vec, h_vec, x_vec);
        case 60 :  h_vec = vld1_s16(h + 56);
                   x_vec = vld1_s16(x + 56);
                   result_vec = vmlal_s16(result_vec, h_vec, x_vec);
        case 56 :  h_vec = vld1_s16(h + 52);
                   x_vec = vld1_s16(x + 52);
                   result_vec = vmlal_s16(result_vec, h_vec, x_vec);
        case 52 :  h_vec = vld1_s16(h + 48);
                   x_vec = vld1_s16(x + 48);
                   result_vec = vmlal_s16(result_vec, h_vec, x_vec);
        case 48 :  h_vec = vld1_s16(h + 44);
                   x_vec = vld1_s16(x + 44);
                   result_vec = vmlal_s16(result_vec, h_vec, x_vec);
        case 44 :  h_vec = vld1_s16(h + 40);
                   x_vec = vld1_s16(x + 40);
                   result_vec = vmlal_s16(result_vec, h_vec, x_vec);
        case 40 :  h_vec = vld1_s16(h + 36);
                   x_vec = vld1_s16(x + 36);
                   result_vec = vmlal_s16(result_vec, h_vec, x_vec);
        case 36 :  h_vec = vld1_s16(h + 32);
                   x_vec = vld1_s16(x + 32);
                   result_vec = vmlal_s16(result_vec, h_vec, x_vec);
        case 32 :  h_vec = vld1_s16(h + 28);
                   x_vec = vld1_s16(x + 28);
                   result_vec = vmlal_s16(result_vec, h_vec, x_vec);
        case 28 :  h_vec = vld1_s16(h + 24);
                   x_vec = vld1_s16(x + 24);
                   result_vec = vmlal_s16(result_vec, h_vec, x_vec);
      }
    }
    else
    { for (i = 28 ; i < ntaps; i = i+4)
      {   h_vec = vld1_s16(h + i);
          x_vec = vld1_s16(x + i);
          result_vec = vmlal_s16(result_vec, h_vec, x_vec);
      }
    }

	/* Reduction operation - add each vector lane result to the sum */
    sum = vgetq_lane_s32(result_vec, 0);
    sum += vgetq_lane_s32(result_vec, 1);
    sum += vgetq_lane_s32(result_vec, 2);
    sum += vgetq_lane_s32(result_vec, 3);

#elif CORTEXM
    /* Run the below code for Cortex-M4 / M7 / M33 */
    if (ntaps <= 24)
    { switch (ntaps)
      {
      case 24 : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      case 22 : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      case 20 : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      case 18 : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      case 16 : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      case 14 : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      case 12 : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      case 10 : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      case 8  : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      case 6  : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      case 4  : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      case 2  : sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      default : break;
      }
    } else
    {
        // do the first 24 in fast mode 
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      for (i = 26 ; i < ntaps; i = i+2)
      {   sum = __SMLAD(*__SIMD32(x)++, *__SIMD32(h)++, sum);
      }
    }
#elif MVE

#else
    sum64 = 0;
    for (i = 0 ; i < ntaps; i++)
    {  sum64 = sum64 + ((*x++) * (*h++));
    }
#endif

	return sum64;
}



/**
  @brief         rate conversion check FS_out / FS_in , compute nb_data_read, nb_data_write
  @param[in]     in_out    XDM buffer
  @return        none
 */

/* - sampling rate converter, with a dedicated processing for the asynchronous to synchronous conversion, 
    to second scratch buffer

    Interpolate L, Decimate M
 
*/
void sigp_stream_converter_process2 (fwd_ssrc_instance *ssrc,
    sigp_stream_converter_format *input_format, sigp_stream_converter_format *output_format, 
    float *inputBuffer, float *outputBuffer, int32_t nSampIn, intPtr_t  *nSampOut)
{
    iidx_t i;
    iidx_t *push = ssrc->memoriesPtr->push;
    iidx_t phase = ssrc->phase;
    iidx_t L = ssrc->nPhases;
    s32_t  firlen = ssrc->firlen;
    s32_t  idxFirmem = ssrc->idxFirmem;
    s32_t  cumulatedPush = 0;
    iidx_t nCopy, nPush;
    iidx_t nRemainSamples = nSampIn;
    samp_f *firmem = ssrc->memoriesPtr->firRam.f32;
    samp_f *fmem1 = firmem + idxFirmem;
    samp_f *fmem2 = inputBuffer;
    samp_f *fmem3 = outputBuffer;

    coef_f *pt_coef = (coef_f *)(ssrc->memoriesPtr->coefReordered.f32) + ssrc->idxCoef;

   do
   {   if (phase >= L-1)
       {   phase = 0;
           pt_coef = ssrc->memoriesPtr->coefReordered.f32;
       } else
       {   phase = phase +1;
       }
       
       nPush = push[phase];
       if (nPush)
       {           
           cumulatedPush += nPush;

           // check for firmem rewind
           if (idxFirmem + cumulatedPush >= ssrc->idxMax)
           {
               fmem1 -= firlen;
               for (i = 0; i < firlen ; i++)
               { (ssrc->memoriesPtr->firRam.f32)[i] = *fmem1++; 
               }
               fmem1 = firmem + firlen;
               idxFirmem = firlen;
           }

           // check for end of input buffer
           if (cumulatedPush > nSampIn)
           {
               // copy the remaining samples and return
               for (i = 0; i < nRemainSamples; i++)
                 *fmem1++ = *fmem2++; 

               // no processing : step back in phase:
               if (phase == 0) phase = L-1;
               else phase = phase -1;

               goto L_endInputBuffer;
           }

           if (ssrc->remainSamp > 0)
           {    
                if (nPush < ssrc->remainSamp)
                {   nCopy = 0;
                    ssrc->remainSamp -= nPush;
                    cumulatedPush -= nPush;
                }
                else
                {   nCopy = nPush - ssrc->remainSamp;
                    cumulatedPush -= ssrc->remainSamp;
                    ssrc->remainSamp = 0;
                }
           }
           else
           {   nCopy = nPush;
           }

           for (i = 0; i < nCopy; i++)
           {  *fmem1++ = *fmem2++;            /* read new data sample */
           }

           nRemainSamples -= nCopy;
       }


       *fmem3 = dotp_f32_16taps(fmem1-firlen, pt_coef, (s16_t)firlen);
       pt_coef += firlen;

       fmem3++;



    } while (nRemainSamples > 0);

    // save context
L_endInputBuffer:    
    ssrc->phase = phase;
    ssrc->idxFirmem = fmem1 - firmem;
    ssrc->idxCoef = pt_coef - ssrc->memoriesPtr->coefReordered.f32;
    ssrc->remainSamp = nRemainSamples;

    *nSampOut = fmem3 - outputBuffer;
}

/**
  @brief         re-interleaving and format conversion to the output buffer check raw_out, domain_out, nchan(out) and intlv_out
  @param[in]     in_out    XDM buffer
  @return        none
 */

/* - raw data conversion, interleaving conversion and copy of the result
*/
void sigp_stream_converter_process3 (sigp_stream_converter_format *output, 
    float *tmpBuf2, float *outBuf)
{
}

#ifdef __cplusplus
}
#endif
 