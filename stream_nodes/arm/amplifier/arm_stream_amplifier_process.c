/* ----------------------------------------------------------------------


        WORK ON GOING



 * Project:      CMSIS Stream
 * Title:        arm_amplifier_process.c
 * Description:  filters
 *
 * $Date:        15 February 2023
 * $Revision:    V0.0.1
 * -------------------------------------------------------------------- */
/*
 * Copyright (C) 2010-2023 ARM Limited or its affiliates. All rights reserved.
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

#include "platform.h"
#ifdef CODE_ARM_STREAM_AMPLIFIER


#ifdef __cplusplus
 extern "C" {
#endif
   

#include <stdint.h>
#include "stream_common_const.h"
#include "stream_common_types.h"
#include "arm_stream_amplifier.h"


  inline int32_t __SSAT(int32_t val, uint32_t sat)
  {
    if ((sat >= 1U) && (sat <= 32U))
    {
      const int32_t max = (int32_t)((1U << (sat - 1U)) - 1U);
      const int32_t min = -1 - max ;
      if (val > max)
      {
        val = max;
      }
      else if (val < min)
      {
        val = min;
      }
    }

    return val;
  }

/**
  @brief         
  @param[in]     pinst      instance of the component
  @param[in/out] pdata      address and size of buffers
  @param[out]    pstatus    execution state (0=processing not finished)
  @return        status     finalized processing
 */
void arm_stream_amplitude_process (arm_amplitude_instance *instance, void *input, void *output /*, intptr_t *nsamp */)
{
    uint8_t slope, ichan;
    int16_t *psrc, *pdst, exp, delayup, delaydown;
    uint32_t format, incsrc, incdst, isamp, W0, W1;
    int32_t current_mantissa, target_mantissa;

    delayup = (int16_t)RD(instance->current_delay, DELAY_UP);
    delaydown = (int16_t)RD(instance->current_delay, DELAY_DOWN);
    format = (instance->channel_fmt);
    W0 = instance->parameters[0];
    W1 = instance->parameters[1];

    /* input */
    psrc = (int16_t *)(input);
    //if (RD(format, INTERLEAVING_FMT) == FMT_INTERLEAVED)
    //{   psrc = psrc + RD(W0, SUBCHANID_W0_IN);
    //    incsrc = 1+ RD(format, NBCHANM1_AMPLI_FMT);
    //}
    //else
    //{   psrc = psrc + RD(format, FRAMESIZE_FMT0_FMT);
    //    incsrc = 1;
    //}
    incsrc = 1;
    /*
        decrease delay until 0 then mute or unmute
    */

    /* output = last arc */
    pdst = (int16_t *)(output);
    //if (RD(format, INTERLEAVING_FMT) == FMT_INTERLEAVED)
    //{   pdst = pdst + RD(param, SUBCHANID_IN);
    //    incdst = 1+ RD(format, NBCHANM1_AMPLI_FMT);
    //}
    //else
    //{   pdst = pdst + RD(format, FRAMESIZE_FMT0_FMT);
    //    incdst = 1;
    //}
    incdst = 1;

    /* prepare the update of the gain */
    slope = (uint8_t)(16 - RD(W0, SLOPE));

    /* example with 0dB = 0x805
       target_mantissa = 0x40000000 
    *  current_mantissa after shift = 0x4000
    *  post-multiplication shift = 14 (9 + target exponent)
    */
    exp = 9 + RD(W0, GAIN_EXPONENT);

    for (isamp = 0; isamp < RD(format, FRAMESIZE_FMT0_FMT); isamp++)
    {   *pdst = 0;
        pdst += incdst;
    }
    

    if (RD(W0, MUTED))
    {   target_mantissa = (RD(W0, GAIN_MANTISSA)) << 23;
    }
    else /* muting is going through ramp-down filter with a faster slope */
    {   target_mantissa = 0;
        slope = (uint8_t)(16 - (RD(W0, SLOPE) >> 1));
    }

    current_mantissa = instance->ampli_current_mantissa;

    for (isamp = 0; isamp < RD(format, FRAMESIZE_FMT0_FMT); isamp++)
    {
        for (ichan = 0; ichan < RD(format, NBCHANM1_AMPLI_FMT); ichan++)
        {   int32_t acc, tmp;
            acc = (*pdst);
            tmp = ((int32_t)(*psrc) * (current_mantissa >> 16)) >> exp;
            acc = acc + tmp;
            *pdst = (int16_t)(__SSAT (acc, 16));
            psrc += incsrc;
            pdst += incdst;

            /* gain slope created with a first order IIR filter */
            current_mantissa += (target_mantissa - current_mantissa) >> slope;
        }
    }

    /* update the filter */
    instance->ampli_current_mantissa = current_mantissa;
}

#ifdef __cplusplus
}
#endif
 
#endif  // #ifndef CODE_ARM_STREAM_AMPLIFIER