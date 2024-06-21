/* ----------------------------------------------------------------------
 * Project:      CMSIS Stream
 * Title:        arm__stream_detector_process.c
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

#ifdef __cplusplus
 extern "C" {
#endif
   

#include "stream_const.h"
#include "stream_types.h"

#define PRINTF 1 // debug

#if PRINTF
#include <stdio.h>
#endif

#include "arm_stream_detector.h"

#include <inttypes.h>
#include <stdbool.h>


/*
    9.	stream_detector
    Operation : provides a boolean output stream from the detection of a rising (of falling) edge above 
    a tunable signal to noise ratio. A time constant in [ms] is used for the detection. 
    A tunable delay allows to maintain the boolean value for a minimum amount of time 
    for debouncing and LED / user-interface).

    Parameters : select rising/falling detection, signal to noise ratio in voltage decibels, 
    time-constant in [ms] for the energy integration time, time-constant to gate the output.
*/


/**
  @brief         Processing function 
  @param[in]     instance     points to an instance of the Biquad cascade structure
  @param[in]     pSrc         points to the block of input data
  @param[out]    pResult      points to the result flag = 0x7FFF when detected, else 0
  @param[in]     inputLength  number of samples to process
  @return        none
 */
void arm_stream_detector_process (arm_detector_instance *pinstance, 
                     int16_t *in, int32_t inputLength, 
                     int16_t *pResult)
{
    int32_t isamp = 0;
    int32_t input_data;
    int32_t DBGZ3;

    while (isamp < inputLength) 
    {
        input_data = ConvertSamp(in[isamp], 15 - (int)(pinstance->config.high_pass_shifter));

        /* high-pass prefilter */
        Z2 = Z1 - DIVBIN(Z1 , SHPF) + input_data;
        Z3 = Z2 - Z1;
        Z1 = Z2;
        DBGZ3 = Z3;
        Z3 = (Z3 < 0) ? (-Z3) : Z3;

        /* z6: raw energy estimation, z7: floor level, z8: envelope */
        Z6 = DIVBIN(Z3, SLPF) + (Z6 - DIVBIN(Z6, SLPF));
        Z8 = DIVBIN(Z6, SFloorPeak) + (Z8 - DIVBIN(Z8, SFloorPeak));
        Z8 = MAX(Z6, Z8);

        /* floor noise update after decimation */
        DECF = decfMASK & (DECF -1);
        if (DECF == 0)
        {   Z7 = DIVBIN(Z6, SFloorPeak) + (Z7 - DIVBIN(Z7, SFloorPeak));
            Z7 = MAX(CLAMP_MIN, MIN(Z7, Z6));
            DECF = decfMASK;
        }

        /* if SNR>THR then increment a first counter */
        if (Z8 > Z7 * (MINIFLOAT2Q31(THR)))
        {   ACCVAD = MIN(CLAMP_MAX, ACCVAD + VADRISE);
        }   /* slow rise, fast fall */
        else
        {   ACCVAD = MAX(CLAMP_MIN, ACCVAD - VADFALL);
        }

        /* if the VAD is confirmed then use a second counter */
        if (ACCVAD > F2Q31(0.1))
        {   FLAG = MIN(CLAMP_MAX, FLAG + VADFALL);
        }   /* fast rise, slow fall */
        else
        {   FLAG = MAX(CLAMP_MIN, FLAG - VADRISE);
        }

        /* signal detected => maintain the decision for some time */
        if (DOWNCOUNTER > 0)
        {   DOWNCOUNTER --;
        }
        if (FLAG > F2Q31(0.5))
        {   DOWNCOUNTER = MINIFLOAT2Q31(RELOADCOUNTER);
        }

        pResult[isamp] = (DOWNCOUNTER > 0) ? 0x7FFF : 0;

#if PRINTF        
        {   
            extern FILE *ptf_trace;
            long x, SD=15-10; 
            x = input_data<<SD;                         fwrite(&x, 1, 4, ptf_trace);    //1
            x = DBGZ3<<SD;                              fwrite(&x, 1, 4, ptf_trace);    //2
            x = Z8<<SD;                                 fwrite(&x, 1, 4, ptf_trace);    //3
            x = ACCVAD;                                 fwrite(&x, 1, 4, ptf_trace);    //4
            x = FLAG;                                   fwrite(&x, 1, 4, ptf_trace);    //5
            x = 0; if (DOWNCOUNTER > 0) x=0x7fffffff;   fwrite(&x, 1, 4, ptf_trace);    //6
        }
#endif     
        isamp++;
    }
}

#ifdef __cplusplus
}
#endif
 