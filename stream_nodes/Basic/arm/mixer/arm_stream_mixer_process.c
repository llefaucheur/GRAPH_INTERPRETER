/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm_stream_mixer_process.c
 * Description:  mixer
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


#ifdef __cplusplus
 extern "C" {
#endif
   
#include "stream_const.h"

#include "stream_types.h"
#include "arm_stream_mixer.h"
#include "dsp/none.h"

/**
  @brief         
  @param[in]     pinst      instance of the component : format of channels + mixing parameters
  @param[in/out] xdm_buf    address and size of buffers
  @param[in/out] narc    
  @return        status     finalized processing
 */
void arm_stream_mixer_process (arm_mixer_instance *instance, stream_xdmbuffer_t *xdm_buf, uint8_t narc)
{
    uint8_t iparam, slope, arcid;
    int16_t *psrc, *pdst, *pdst2, exp;
    uint32_t W0, W1, W2, W3, format, incsrc, incdst, isamp;
    int32_t current_mantissa, target_mantissa;

    W0 = instance->parameters[0];
    W1 = instance->parameters[1];
    pdst2 = 0;

    for (iparam = 0; iparam < MAXMIXERINDEXES; iparam ++)
    {   
        W2 = instance->parameters[2+2*iparam];
        W3 = instance->parameters[3+2*iparam];
        format = instance->channel_fmt[iparam];

        /* gain = 0 <=> end of mixing */
        if (RD(W2, GAIN_MANTISSA_W2) == 0) 
        {   break;
        }

        /* input */
        arcid = RD(W2, MIXER_ARCID_IN_W2);
        psrc = (int16_t *)((xdm_buf + arcid)->address);
        if (RD(format, INTERLEAVING_FMT) == FMT_INTERLEAVED)
        {   psrc = psrc + RD(W2, SUBCHANID_IN_W2);
            incsrc = 1+ RD(format, NBCHANM1_MIXER_FMT);
        }
        else
        {   psrc = psrc + RD(format, FRAMESIZE_FMT0_FMT);
            incsrc = 1;
        }

        /* output = last arc */
        pdst = (int16_t *)((xdm_buf + narc)->address);
        if (RD(format, INTERLEAVING_FMT) == FMT_INTERLEAVED)
        {   pdst = pdst + RD(W2, SUBCHANID_IN_W2);
            incdst = 1+ RD(format, NBCHANM1_MIXER_FMT);
        }
        else
        {   pdst = pdst + RD(format, FRAMESIZE_FMT0_FMT);
            incdst = 1;
        }
        pdst2 = pdst;

        /* prepare the update of the gain */
        slope = 16 - RD(W0, SLOPE_W0);

        /* example with 0dB = 0x805
           target_mantissa = 0x40000000 
        *  current_mantissa after shift = 0x4000
        *  post-multiplication shift = 14 (9 + target exponent)
        */

        /* clear the output buffer before cumulating */
        if (iparam == 0)
        {   for (isamp = 0; isamp < RD(format, FRAMESIZE_FMT0_FMT); isamp++)
            {   *pdst = 0;
                pdst += incdst;
            }
        }

        exp = 9 + RD(W2, GAIN_EXPONENT_W2);

        if (RD(W3,MUTED_CHANID_W3))
        {   target_mantissa = (RD(W2, GAIN_MANTISSA_W2)) << 23;
        }
        else /* muting is going through ramp-down filter with a faster slope */
        {   target_mantissa = 0;
            slope = 16 - (RD(W0, SLOPE_W0) >> 1);
        }
        current_mantissa = (instance->mixer_current_mantissa)[iparam];

        for (isamp = 0; isamp < RD(format, FRAMESIZE_FMT0_FMT); isamp++)
        {   int32_t acc, tmp;
            acc = (*pdst);
            tmp = ((int32_t)(*psrc) * (current_mantissa >> 16)) >> exp;
            acc = acc + tmp;
            *pdst = __SSAT (acc, 16);
            psrc += incsrc;
            pdst += incdst;

            /* gain slope created with a first order IIR filter */
            current_mantissa += (target_mantissa - current_mantissa) >> slope;
        }

        /* update the filter */
        (instance->mixer_current_mantissa)[iparam] = current_mantissa;
    }

    /* apply the output gain */
    current_mantissa = RD(W0, OUTGAIN_MANTISSA_W0);
    exp = 9 + RD(W0, OUTGAIN_EXPONENT_W0);

    for (isamp = 0; isamp < RD(format, FRAMESIZE_FMT0_FMT); isamp++)
    {   int32_t tmp;
        tmp = ((int32_t)(*pdst2) * (current_mantissa >> 16)) >> exp;
        *pdst2 = __SSAT (tmp, 16);
        pdst2 += incdst;
    }
}
#ifdef __cplusplus
}
#endif
 